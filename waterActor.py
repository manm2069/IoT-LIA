import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO

# Set up the GPIO mode
GPIO.setmode(GPIO.BCM)

# Define and intialize pins
GPIO.setup(18, GPIO.OUT)

# Define the MQTT broker and topic
broker_address = "localhost"  # Replace with the Raspberry Pi's IP if not running locally
topic = "tank/led"  # Replace with the desired MQTT topic

# Callback functions for MQTT client
def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker with result code " + str(rc))
    client.subscribe(topic)

def on_message(client, userdata, message):
    print(f"Received message on topic '{message.topic}': {message.payload.decode()}")
    tank_state = str(message.payload.decode("utf-8"))

    if tank_state == "open":
        GPIO.output(18, 1)
    elif tank_state == "close":
        GPIO.output(18, 0)

# Create an MQTT client
client = mqtt.Client()

# Set up callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(broker_address)

# Start the MQTT client loop to receive messages
client.loop_forever()

# Allow the LED to be turned on and off
client.open_tank = on_message
