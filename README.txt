---

# **Concurrent Stair Lighting System - Wiring Instructions**

This guide provides wiring instructions for setting up the **concurrent stair lighting system** using an **ESP32** microcontroller, **infrared motion sensors**, and **relay modules**.

---

## **1. Components Required**
### **Microcontroller**  
- **ESP32** (or compatible development board)  

### **Sensors**  
- **2x PIR Sensors** (Infrared motion sensors)  
  - **Sensor at the top of the stairs**  
  - **Sensor at the bottom of the stairs**  

### **Relay Module**  
- **15-channel relay module** (to control stair lights)  

### **Lighting**  
- **LED strip lights / AC bulbs** (depending on application)  

### **Power Supply**  
- **5V DC** for relays  
- **ESP32 Power: 5V USB or 3.3V regulator**  

---

## **2. Wiring Diagram**
### **ESP32 Pinout**
| ESP32 Pin | Connected To | Description |
|-----------|-------------|-------------|
| `GPIO34`  | **Top Sensor Output** | Reads signal from top PIR sensor |
| `GPIO35`  | **Bottom Sensor Output** | Reads signal from bottom PIR sensor |
| `GPIO4`   | **Relay 1 (Step 1)** | Controls first relay (step 1 light) |
| `GPIO5`   | **Relay 2 (Step 2)** | Controls second relay |
| `GPIO13`  | **Relay 3 (Step 3)** | Controls third relay |
| `GPIO14`  | **Relay 4 (Step 4)** | Controls fourth relay |
| `GPIO16`  | **Relay 5 (Step 5)** | Controls fifth relay |
| `GPIO17`  | **Relay 6 (Step 6)** | Controls sixth relay |
| `GPIO18`  | **Relay 7 (Step 7)** | Controls seventh relay |
| `GPIO19`  | **Relay 8 (Step 8)** | Controls eighth relay |
| `GPIO21`  | **Relay 9 (Step 9)** | Controls ninth relay |
| `GPIO22`  | **Relay 10 (Step 10)** | Controls tenth relay |
| `GPIO23`  | **Relay 11 (Step 11)** | Controls eleventh relay |
| `GPIO25`  | **Relay 12 (Step 12)** | Controls twelfth relay |
| `GPIO26`  | **Relay 13 (Step 13)** | Controls thirteenth relay |
| `GPIO27`  | **Relay 14 (Step 14)** | Controls fourteenth relay |
| `GPIO33`  | **Relay 15 (Step 15)** | Controls fifteenth relay |

---

## **3. Wiring Details**
### **A. Connecting the PIR Sensors**
Each **PIR motion sensor** has **three pins**:
- **VCC** → **Connect to 3.3V or 5V (depending on sensor specs)**
- **GND** → **Connect to ESP32 GND**
- **OUT** → **Connect to ESP32 GPIO34 (Top Sensor) or GPIO35 (Bottom Sensor)**  

> **Note:** If using **HC-SR501** PIR sensors, set them to "Repeat Trigger Mode" for best performance.

---

### **B. Connecting the Relay Module**
Each **relay module** has **three input pins per relay**:
- **VCC** → **Connect to 5V (from external power supply)**
- **GND** → **Connect to ESP32 GND**
- **IN1 - IN15** → **Connect to respective ESP32 GPIO pins (as listed in the table above)**

**Relay Power Considerations:**  
- If using an **optocoupler relay module**, ensure **JD-VCC is powered separately** from ESP32 to avoid voltage drops.  
- If controlling **AC lights**, use a **proper relay-rated power source** and ensure safe wiring.  

> **Warning:** If switching **high-voltage AC loads**, use a **proper relay module** with necessary **isolation and precautions**.

---

### **C. Powering the ESP32**
- The **ESP32** can be powered via:  
  - **Micro-USB (5V via computer or adapter)**  
  - **5V from a regulated power supply**  
  - **3.3V via a dedicated voltage regulator**  

> **Note:** If powering both ESP32 and relays from the same power source, ensure sufficient current (at least **2A at 5V**) to prevent brownouts.

---

## **4. Final Checks**
✅ Ensure **PIR sensors** have stable **power and ground connections**.  
✅ Check **relay module connections** to match the GPIO pins in the **code**.  
✅ Use **proper power sources** to prevent ESP32 restarts or relay failures.  
✅ If controlling **AC loads**, use **safe wiring practices**.  

---

## **5. Troubleshooting**
| Issue | Possible Cause | Solution |
|-------|--------------|----------|
| Lights do not turn on | Sensor wiring issue | Check PIR sensor power and OUT pin connections |
| Lights turn off too quickly | Sensors not re-triggering | Ensure PIR sensors are in **Repeat Mode** |
| ESP32 resets randomly | Power instability | Use a **dedicated 5V 2A+ power source** |
| Relays not activating | GPIO mismatch | Ensure ESP32 pins match those in **code** |

---

## **6. Additional Notes**
- **Customize GPIO pins in the code** if needed.  
- Ensure **PIR sensors** have a **clear line of sight** for detection.  
- If using **different relays**, adjust **HIGH/LOW logic** accordingly.  

---

### **Now you’re ready to power on your stair lighting system! 🚀**  
If you have any issues, check your wiring and sensor placement.  

---