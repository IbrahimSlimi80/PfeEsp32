# 🌡️ SmartClimate ESP32 Project

SmartClimate is an IoT project using an ESP32, DHT11 sensor, and Firebase Realtime Database to collect and monitor real-time temperature and humidity data. Data can be controlled remotely via a Firebase variable to start or stop the sensor readings.

---

## 🚀 Features

- 🌡️ Reads Temperature and Humidity using a DHT11 sensor.
- 📡 Sends data to Firebase Realtime Database every 15 seconds.
- 🔒 Uses Firebase Email/Password Authentication for secure access.
- 🛑 Remotely control data transmission using a Firebase `control` variable (`send` or `stop`).
- 🕒 Data includes timestamp via NTP for accurate tracking.
- 📱 Integrates with an Android app for live data visualization (optional frontend).

---

## 🧰 Requirements

- ESP32 board
- DHT11 sensor
- Arduino IDE
- Firebase project (Realtime Database + Authentication enabled)
- Wi-Fi access
- Required libraries:
  - `Firebase ESP Client`
  - `DHT sensor library`
  - `Adafruit Unified Sensor`
  - `WiFi.h` (built-in)

---

## 🔧 Setup Instructions

### 1. Clone the project

```bash
git clone https://github.com/yourusername/SmartClimate.git

2. Update Configuration in the Code

Open the code and update the following values:

// WiFi credentials
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Firebase credentials
#define API_KEY "Your Firebase Web API Key"
#define DATABASE_URL "https://your-project-id.firebaseio.com/"

// Firebase user credentials (used to authenticate to Firebase)
#define USER_EMAIL "yourfirebaseuser@example.com"
#define USER_PASSWORD "yourUserPassword"

3. Firebase Setup
✅ Enable Email/Password Authentication

Go to Firebase Console > Authentication > Sign-in method > Enable Email/Password.
✅ Create a Firebase user

Go to Users > Add User, and use the email/password above.
✅ Realtime Database Rules

Paste the following rules in your Database > Rules tab:

{
  "rules": {
    ".read": true,
    ".write": "auth != null",  // Only authenticated users can write
    "control": {
      ".read": true,
      ".write": true
    },
    "sensors": {
      ".read": true,
      ".write": "auth != null",
      "temperature": {
        ".indexOn": ["datetime"]
      },
      "humidity": {
        ".indexOn": ["datetime"]
      }
    }
  }
}

✅ Create the Control Variable

In Realtime Database, manually create a control key (at root level) with value "send" or "stop" to control data flow.
🛠️ How It Works

    The ESP32 connects to your Wi-Fi and authenticates to Firebase.

    Every 15 seconds (if control = send), it:

        Reads temperature and humidity.

        Gets the current timestamp using NTP.

        Pushes the data to:

            sensors/temperature/

            sensors/humidity/

    You can change control in Firebase to "stop" to pause the sensor.

🧪 Sample Firebase Structure

{
  "control": "send",
  "sensors": {
    "temperature": {
      "-Mp123abc456": {
        "value": 22.5,
        "datetime": "2025-04-18 10:34:21"
      }
    },
    "humidity": {
      "-Mp123def789": {
        "value": 48.2,
        "datetime": "2025-04-18 10:34:21"
      }
    }
  }
}

🐞 Debugging Tips

    If you see connection refused, check:

        Wi-Fi connection.

        Firebase credentials and URL.

        That the user is created in Firebase.

        .write rules for the path.

📱 Android Integration (Optional)

This project can be used with a companion Android app that reads data from the Firebase Realtime Database and displays it using charts.
✍️ License

MIT License. Feel free to use, modify, and share this project.
💬 Support

For questions or collaboration, feel free to open an issue or contact me.
