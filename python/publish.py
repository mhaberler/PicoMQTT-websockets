import paho.mqtt.client as mqtt
import time
import logging

logging.basicConfig(level=logging.DEBUG)


def on_connect(mqttc, obj, flags, reason_code, properties):
    print("reason_code: "+str(reason_code))

def on_message(mqttc, obj, msg):
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))

def on_subscribe(mqttc, obj, mid, reason_code_list, properties):
    print("Subscribed: "+str(mid)+" "+str(reason_code_list))

def on_log(mqttc, obj, level, string):
    print(string)


def on_publish(cclient, userdata, mid, reason_code, properties):
    print("Message published")

# Create an MQTT client instance
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, transport="websockets", client_id="pypub", clean_session=True)
client.enable_logger()

# Set up callback functions
client.on_connect = on_connect
client.on_publish = on_publish

# Connect to the MQTT broker
broker_address = "172.16.0.217"
port = 8883
client.connect(broker_address, port, 60)

# Publish a message to a topic
topic = "blah"

n = 0
# time.sleep(10)
client.loop_start()

while True:
    client.publish(topic, f"Hello, MQTT! {n}")
    client.loop()
    time.sleep(1)
    n += 1

