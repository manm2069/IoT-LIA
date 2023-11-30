import paho.mqtt.client as mqtt
import json


class Irrigation_Controller:
    configuration = []
    client = None
    mqtt_data = {}
#{ "topic":value, "topic2":value2 }

    def configure(filename):
        Irrigation_Controller.client = mqtt.Client()
        #load the configuration from a file
        with open(filename,'r') as file:
            Irrigation_Controller.configuration = json.load(file)
        # print(configuration)
        # connecting to the MQTT broker
        Irrigation_Controller.client.on_message = Irrigation_Controller.on_message
        Irrigation_Controller.client.connect("localhost",1883)
        # subscribe to the appropriate topics (the ones from the conditions)
        for rule in Irrigation_Controller.configuration:
            for condition in rule["conditions"]:
                Irrigation_Controller.client.subscribe(condition["topic"])
                print(condition["topic"])

    def run():
        # start the MQTT client loop
        print("run method before loop_forever")
        Irrigation_Controller.client.loop_forever()
        print("run method after loop_forever")

    def on_message(client, userdata, message):
        value = int(message.payload.decode("utf-8"))
        topic = message.topic
        Irrigation_Controller.mqtt_data[topic] = value
#        print(f"Received message on topic '{topic}': {value}")
        # e.g., Irrigation_Controller.mqtt_data["system/temperature"] = 25
        Irrigation_Controller.run_rules()

    def run_rules():
        for rule in Irrigation_Controller.configuration:
            conditions_met = all(Irrigation_Controller.evaluate_condition(Irrigation_Controller.mqtt_data, condition) for condition in rule["conditions"])

            if conditions_met:
                # todo: call the events to happen
                for message in rule["results"]:
                    Irrigation_Controller.client.publish(message["topic"],message["value"])
#                    print(f"Sending {message[value]} to {message[topic]}")

    def evaluate_condition(data, condition):
#        {"topic":"soil/moisture","comparison":"<","value":30},
        topic = condition["topic"]
        value = data.get(topic,None) # not getting a None when something is missing
        if value == None:
            return False

        comparison = condition["comparison"]
        if comparison == "<":
            return value < condition["value"]
        elif comparison == "<=":
            return value <= condition["value"]
        elif comparison == "==":
            return value == condition["value"]
        elif comparison == "!=":
            return value != condition["value"]
        elif comparison == ">":
            return value > condition["value"]
        elif comparison == ">=":
            return value >= condition["value"]
        else:
            return False

def main():
    Irrigation_Controller.configure("config.json")
    Irrigation_Controller.run()

if __name__ == "__main__":
    main()
