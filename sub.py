from paho.mqtt import client as mqtt_client
import requests
import time

broker = 'broker.emqx.io'
port = 1883
topic1 = "Latitude"
topic2 = "Longitude"
topic3 = "mph"
client_id = f'python-mqtt-Client-2'

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        if(msg.topic == 'mph'):    
            URL = 'https://api.thingspeak.com/update?api_key=69DC41HZOHXHL7D0&field1=0'+str(msg.payload.decode())
            response=requests.get(URL)
            print(response)
    client.subscribe(topic1)
    client.subscribe(topic2)
    client.subscribe(topic3)
    client.on_message = on_message
    
def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()
    
if __name__ == '__main__':
    run()
