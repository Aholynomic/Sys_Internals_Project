#!/usr/bin/python3

from gpiozero import LED
from time import sleep
import boto3
import random

mylines = []
data = []

# initialise GPIO pins for red and green leds
# green - GPIO7
# red - GPIO20
green_led = LED(7)
red_led = LED(20)

red_led.on()

print("Red LED ON\n")

# use existing table
dynamodb_client = boto3.resource("dynamodb")
table = dynamodb_client.Table('CMP408_Network_Logs')

# open log file
with open("test.log", "r") as logfile:
	for line in logfile:
		mylines.append(line)

logfile.close()

# get the data seperated by the * symbols
for line in mylines:
	if not line:
		continue
	
	columns = [col.strip() for col in line.split("*") if col]
	data.append(columns)	

sleep(5)

print("Inserting Data to Database. May take a while...")

# iterate through the multidimensional array 
# and set variables manually
for row in data:
	uid = row[0] 
	sys_call = row[1]
	sip = row[2]
	sp = row[3]
	dip = row[4]
	dp = row[5]
	table.put_item(
		Item= {	
			# primary key is random (1 - 999,999)
			'Table ID': random.randrange(1,999999),
			'UID': uid,
			'System Call': sys_call,
			'Source IP Address': sip,
			'Source Port Number': sp,
			'Destination IP Address': dip,
			'Destination Port Number': dp
		}
	)

red_led.off()

print("Red LED OFF\n")

sleep(5)

green_led.on()

print("Green LED ON\n")
print("Data Sent to Cloud\n")

sleep(5)

green_led.off()
