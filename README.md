# MQTT + NTP Server Setup (Ubuntu)

## 1. Install Mosquitto (MQTT Broker)

Update system:

```bash
sudo apt update
```

Install Mosquitto and CLI tools:

```bash
sudo apt install mosquitto mosquitto-clients
```

Enable Mosquitto to start automatically on boot:

```bash
sudo systemctl enable mosquitto
```

Edit Mosquitto configuration:

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

```conf
# /etc/mosquitto/mosquitto.conf file:
persistence true
persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d

listener 1883
allow_anonymous true
```

Restart Mosquitto:

```bash
sudo systemctl restart mosquitto
```

### Test Mosquitto

**Terminal 1 — Subscriber:**

```bash
mosquitto_sub -h localhost -t weather/data
```

**Terminal 2 — Publisher:**

```bash
mosquitto_pub -h localhost -t weather/data -m "hello world"
```

---

## 2. Install NTP Server (Chrony)

Update system:

```bash
sudo apt update
```

Install Chrony:

```bash
sudo apt install chrony
sudo systemctl enable chrony
```

Edit configuration:

```bash
sudo nano /etc/chrony/chrony.conf
```

```conf
# /etc/chrony/chrony.conf file
# Use local system clock as fallback source
server 127.127.1.0

# Mark the local clock as a valid NTP source
local stratum 10

# Allow LAN clients
allow 192.168.0.0/24

driftfile /var/lib/chrony/chrony.drift
rtcsync
makestep 1 3
logdir /var/log/chrony
```

Restart Chrony:

```bash
sudo systemctl restart chrony
```

Test the NTP server:

```bash
chronyc tracking
```
