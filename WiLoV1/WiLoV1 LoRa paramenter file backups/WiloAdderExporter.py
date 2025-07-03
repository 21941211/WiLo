import os
import re
import csv

def get_latest_wilo_folder(current_dir):
    wilo_folders = [f for f in os.listdir(current_dir) if re.match(r'WiLo-\d+', f)]
    
    if not wilo_folders:
        return None, 0, "70B3D57ED0000000"
    
    latest_folder = max(wilo_folders, key=lambda x: int(re.search(r'\d+', x).group()))
    latest_number = int(re.search(r'\d+', latest_folder).group())
    
    parameters_file = os.path.join(current_dir, latest_folder, "parameters.txt")
    latest_deveui = "70B3D57ED0000000"
    
    if os.path.exists(parameters_file):
        with open(parameters_file, "r") as file:
            for line in file:
                if line.startswith("DEVEUI="):
                    latest_deveui = line.strip().split("=")[1]
                    break
    
    return latest_folder, latest_number, latest_deveui

def increment_deveui(deveui):
    deveui_int = int(deveui, 16) + 1
    return f"{deveui_int:016X}"  # Format as uppercase 16-character hex

def create_new_wilo_folder(current_dir, new_number, deveui):
    new_folder_name = f"WiLo-{new_number:03d}"
    new_folder_path = os.path.join(current_dir, new_folder_name)
    os.makedirs(new_folder_path, exist_ok=True)
    
    parameters_file = os.path.join(new_folder_path, "parameters.txt")
    with open(parameters_file, "w") as file:
        file.write(f"DEVID={new_number}\n")
        file.write(f"APPEUI=0000000000000000\n")
        file.write(f"DEVEUI={deveui}\n")
        file.write(f"APPKEY=0432982B97D96E80C3E3A5986F98901D\n")
    
    print(f"Created {new_folder_name} with parameters.txt")

def export_to_ttn_csv(current_dir):
    csv_file = os.path.join(current_dir, "ttn_devices.csv")
    fieldnames = ["id", "dev_eui", "join_eui", "frequency_plan_id", "lorawan_version", "lorawan_phy_version", "app_key"]
    
    with open(csv_file, "w", newline='') as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        
        for folder in sorted(os.listdir(current_dir)):
            if re.match(r'WiLo-\d+', folder):
                param_file = os.path.join(current_dir, folder, "parameters.txt")
                if os.path.exists(param_file):
                    with open(param_file, "r") as pfile:
                        params = {line.split("=")[0]: line.split("=")[1].strip() for line in pfile if "=" in line}
                    writer.writerow({
                        "id": f"wilo-{int(params.get('DEVID')):03d}",
                        "dev_eui": params.get("DEVEUI"),
                        "join_eui": "0000000000000000",
                        "frequency_plan_id": "EU_863_870_TTN",
                        "lorawan_version": "MAC_V1_0_2",
                        "lorawan_phy_version": "RP002_V1_0_2",
                        "app_key": params.get("APPKEY")
                    })
    
    print(f"Exported LoRa parameters to {csv_file}")

def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    
    latest_folder, latest_number, latest_deveui = get_latest_wilo_folder(current_dir)
    new_number = latest_number + 1
    new_deveui = increment_deveui(latest_deveui)
    
    create_new_wilo_folder(current_dir, new_number, new_deveui)
    export_to_ttn_csv(current_dir)

if __name__ == "__main__":
    main()
