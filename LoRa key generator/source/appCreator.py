import os
import re
import csv
import tkinter as tk
from tkinter import messagebox, filedialog, simpledialog

class WiLoApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("WiLo Device Generator")
        self.geometry("450x400")

        # Input fields
        self.devid_var = tk.StringVar()
        self.appeui_var = tk.StringVar()
        self.deveui_var = tk.StringVar()
        self.appkey_var = tk.StringVar()
        self.num_additional_var = tk.IntVar(value=0)
        self.export_path = None

        self.create_widgets()

        # Track created devices info for CSV export
        self.created_devices = []

    def create_widgets(self):
        row = 0
        tk.Label(self, text="Enter DEVID (integer):").grid(row=row, column=0, sticky="w", padx=10, pady=5)
        tk.Entry(self, textvariable=self.devid_var).grid(row=row, column=1, pady=5)

        row += 1
        tk.Label(self, text="Enter APPEUI (16 hex chars):").grid(row=row, column=0, sticky="w", padx=10, pady=5)
        tk.Entry(self, textvariable=self.appeui_var).grid(row=row, column=1, pady=5)

        row += 1
        tk.Label(self, text="Enter DEVEUI (16 hex chars):").grid(row=row, column=0, sticky="w", padx=10, pady=5)
        tk.Entry(self, textvariable=self.deveui_var).grid(row=row, column=1, pady=5)

        row += 1
        tk.Label(self, text="Enter APPKEY (32 hex chars):").grid(row=row, column=0, sticky="w", padx=10, pady=5)
        tk.Entry(self, textvariable=self.appkey_var).grid(row=row, column=1, pady=5)

        row += 1
        tk.Label(self, text="Number of additional devices to create:").grid(row=row, column=0, sticky="w", padx=10, pady=5)
        tk.Entry(self, textvariable=self.num_additional_var).grid(row=row, column=1, pady=5)

        row += 1
        btn_export_folder = tk.Button(self, text="Select Export Folder", command=self.select_export_folder)
        btn_export_folder.grid(row=row, column=0, columnspan=2, pady=10)

        row += 1
        btn_create = tk.Button(self, text="Create Devices & Export CSV", command=self.create_devices)
        btn_create.grid(row=row, column=0, columnspan=2, pady=10)

    def select_export_folder(self):
        folder = filedialog.askdirectory(title="Select Export Folder")
        if folder:
            self.export_path = folder
            messagebox.showinfo("Export Folder Selected", f"Export folder set to:\n{folder}")
        else:
            messagebox.showwarning("No folder selected", "Please select a folder to export.")

    def validate_hex(self, value, length):
        return len(value) == length and all(c in "0123456789abcdefABCDEF" for c in value)

    def increment_deveui(self, deveui_hex, increment=1):
        num = int(deveui_hex, 16)
        num += increment
        return f"{num:016X}"

    def create_devices(self):
        # Validate inputs
        try:
            devid_start = int(self.devid_var.get())
            if devid_start < 1:
                raise ValueError("DEVID must be >= 1")
        except Exception as e:
            messagebox.showerror("Input Error", f"Invalid DEVID: {e}")
            return

        appeui = self.appeui_var.get().strip()
        deveui = self.deveui_var.get().strip()
        appkey = self.appkey_var.get().strip()
        num_additional = self.num_additional_var.get()

        if not self.validate_hex(appeui, 16):
            messagebox.showerror("Input Error", "APPEUI must be exactly 16 hex characters.")
            return

        if not self.validate_hex(deveui, 16):
            messagebox.showerror("Input Error", "DEVEUI must be exactly 16 hex characters.")
            return

        if not self.validate_hex(appkey, 32):
            messagebox.showerror("Input Error", "APPKEY must be exactly 32 hex characters.")
            return

        if not self.export_path:
            messagebox.showerror("No Export Folder", "Please select an export folder before creating devices.")
            return

        # Check if any WiLo folders exist already in export folder
        existing = [f for f in os.listdir(self.export_path) if re.match(r"WiLo-\d{3}", f)]
        if existing:
            if not messagebox.askyesno(
                "Existing WiLo folders found",
                "There are existing WiLo folders in the export directory. Continue and add new devices?"
            ):
                return

        self.created_devices.clear()

        total_devices = 1 + num_additional
        current_devid = devid_start
        current_deveui_int = int(deveui, 16)

        for i in range(total_devices):
            folder_name = f"WiLo-{current_devid:03d}"
            folder_path = os.path.join(self.export_path, folder_name)
            os.makedirs(folder_path, exist_ok=True)

            current_deveui_hex = f"{current_deveui_int:016X}"

            param_file = os.path.join(folder_path, "parameters.txt")
            with open(param_file, "w") as f:
                f.write(f"DEVID={current_devid}\n")
                f.write(f"APPEUI={appeui.upper()}\n")
                f.write(f"DEVEUI={current_deveui_hex}\n")
                f.write(f"APPKEY={appkey.upper()}\n")

            self.created_devices.append({
                "DEVID": current_devid,
                "APPEUI": appeui.upper(),
                "DEVEUI": current_deveui_hex,
                "APPKEY": appkey.upper()
            })

            current_devid += 1
            current_deveui_int += 1

        messagebox.showinfo("Success", f"Created {total_devices} WiLo device folders with parameters.")

        # Prompt user to export CSV
        if messagebox.askyesno("Export CSV?", "Do you want to export all created devices to a CSV file?"):
            self.export_csv()

    def export_csv(self):
        if not self.export_path or not self.created_devices:
            messagebox.showerror("Error", "No devices to export or no export folder set.")
            return

        csv_path = os.path.join(self.export_path, "ttn_devices.csv")
        fieldnames = ["id", "dev_eui", "join_eui", "frequency_plan_id", "lorawan_version", "lorawan_phy_version", "app_key"]

        with open(csv_path, "w", newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            for d in self.created_devices:
                writer.writerow({
                    "id": f"wilo-{d['DEVID']:03d}",
                    "dev_eui": d["DEVEUI"],
                    "join_eui": "0000000000000000",
                    "frequency_plan_id": "EU_863_870_TTN",
                    "lorawan_version": "MAC_V1_0_2",
                    "lorawan_phy_version": "RP002_V1_0_2",
                    "app_key": d["APPKEY"]
                })

        messagebox.showinfo("CSV Exported", f"CSV exported successfully to:\n{csv_path}")


if __name__ == "__main__":
    app = WiLoApp()
    app.mainloop()
