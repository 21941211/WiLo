import os
import re
import csv

def get_existing_wilo_folders(devices_dir):
    pattern = re.compile(r'WiLoV2-(\d{3})')
    folders = []

    for folder in os.listdir(devices_dir):
        match = pattern.match(folder)
        if match:
            folders.append(int(match.group(1)))

    return sorted(folders)

def prompt_input(prompt_text, validator=None):
    while True:
        response = input(f"{prompt_text}: ").strip()
        if validator is None or validator(response):
            return response
        print("Invalid input. Please try again.")

def validate_hex(value, length):
    return re.fullmatch(rf"[0-9A-Fa-f]{{{length}}}", value) is not None

def create_wilo_folder(devices_dir, devid, appeui, deveui, appkey):
    folder_name = f"WiLoV2-{devid:03d}"
    folder_path = os.path.join(devices_dir, folder_name)
    os.makedirs(folder_path, exist_ok=True)

    parameters_file = os.path.join(folder_path, "parameters.txt")
    with open(parameters_file, "w") as f:
        f.write(f"DEVID={devid}\n")
        f.write(f"APPEUI={appeui.upper()}\n")
        f.write(f"DEVEUI={deveui.upper()}\n")
        f.write(f"APPKEY={appkey.upper()}\n")

    print(f"✅ Created {folder_name}")

def increment_deveui(deveui):
    return f"{int(deveui, 16) + 1:016X}"

def export_to_ttn_csv(devices_dir):
    csv_file = os.path.join(devices_dir, "ttn_devices.csv")
    fieldnames = ["id", "dev_eui", "join_eui", "frequency_plan_id", "lorawan_version", "lorawan_phy_version", "app_key"]

    with open(csv_file, "w", newline='') as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()

        for folder in sorted(os.listdir(devices_dir)):
            if re.match(r'WiLoV2-\d+', folder):
                param_file = os.path.join(devices_dir, folder, "parameters.txt")
                if os.path.exists(param_file):
                    with open(param_file, "r") as pfile:
                        params = {line.split("=")[0]: line.split("=")[1].strip() for line in pfile if "=" in line}
                    writer.writerow({
                        "id": f"wilo-{int(params.get('DEVID')):03d}",
                        "dev_eui": params.get("DEVEUI"),
                        "join_eui": params.get("APPEUI", "0000000000000000"),
                        "frequency_plan_id": "EU_863_870_TTN",
                        "lorawan_version": "MAC_V1_0_2",
                        "lorawan_phy_version": "RP002_V1_0_2",
                        "app_key": params.get("APPKEY")
                    })

    print(f"\n📄 Exported LoRa parameters to: {csv_file}")

def main():
    print("=== WiLoV2 Configuration Utility ===\n")
    script_dir = os.path.dirname(os.path.abspath(__file__))
    devices_dir = os.path.join(script_dir, "devices")
    os.makedirs(devices_dir, exist_ok=True)

    existing = get_existing_wilo_folders(devices_dir)

    if existing:
        print(f"Found {len(existing)} existing WiLoV2 folders. Latest is WiLoV2-{existing[-1]:03d}")
        choice = prompt_input("Do you want to continue from the last device ID? (y/n)", lambda x: x.lower() in ["y", "n"])
        if choice.lower() == 'y':
            start_devid = existing[-1] + 1
        else:
            start_devid = int(prompt_input("Enter starting DEVID (e.g. 1)", lambda x: x.isdigit()))
    else:
        start_devid = int(prompt_input("Enter starting DEVID (e.g. 1)", lambda x: x.isdigit()))

    base_devid = start_devid

    # Prompt for parameters (required, no defaults)
    appeui = prompt_input("Enter APPEUI (16 hex digits)", lambda x: validate_hex(x, 16))
    deveui = prompt_input("Enter starting DEVEUI (16 hex digits)", lambda x: validate_hex(x, 16))
    appkey = prompt_input("Enter APPKEY (32 hex digits)", lambda x: validate_hex(x, 32))

    # Create the first folder
    create_wilo_folder(devices_dir, base_devid, appeui, deveui, appkey)

    num_extra = int(prompt_input("How many additional WiLoV2 devices would you like to create?", lambda x: x.isdigit()))

    current_deveui = deveui
    for i in range(1, num_extra + 1):
        current_devid = base_devid + i
        current_deveui = increment_deveui(current_deveui)
        create_wilo_folder(devices_dir, current_devid, appeui, current_deveui, appkey)

    # Prompt to export
    export = prompt_input("\n📤 Would you like to export all WiLoV2 devices to a CSV file? (y/n)", lambda x: x.lower() in ["y", "n"])
    if export.lower() == 'y':
        export_to_ttn_csv(devices_dir)
    else:
        print("Skipped CSV export.")

    print("\n🎉 Done.")

if __name__ == "__main__":
    main()
