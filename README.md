# Superfanzy

## MQTT Topics
Status
```bash
mosquitto_sub -h 172.16.247.230 -p 1883 -t "/fanctl/status"
```
PWM Set duty-cycle
```bash
mosquitto_pub -h 172.16.247.230 -p 1883 -t "/fanctl/control/{1-4}/PWM"
```
