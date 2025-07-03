import os
import re

def get_latest_wilo_folder(current_dir):
    wilo_folders = [f for f in os.listdir(current_dir) if re.match(r'WiLo\d+', f)]
    
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
    new_folder_name = f"WiLo{new_number}"
    new_folder_path = os.path.join(current_dir, new_folder_name)
    os.makedirs(new_folder_path, exist_ok=True)
    
    parameters_file = os.path.join(new_folder_path, "parameters.txt")
    with open(parameters_file, "w") as file:
        file.write(f"DEVID={new_number}\n")
        file.write(f"APPEUI=0000000000000000\n")
        file.write(f"DEVEUI={deveui}\n")
        file.write(f"APPKEY=A385DDAADBF7F168509454D431994614\n")
    
    print(f"Created {new_folder_name} with parameters.txt")

def main():
    current_dir = os.path.dirname(os.path.abspath(__file__))
    
    latest_folder, latest_number, latest_deveui = get_latest_wilo_folder(current_dir)
    new_number = latest_number + 1
    new_deveui = increment_deveui(latest_deveui)
    
    create_new_wilo_folder(current_dir, new_number, new_deveui)

if __name__ == "__main__":
    main()
