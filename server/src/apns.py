from apns2.credentials import TokenCredentials
from apns2.client import APNsClient
from apns2.payload import Payload

import collections
import json
import os
from dotenv import load_dotenv

load_dotenv()


token_credentials = TokenCredentials(
	auth_key_path=os.getenv('APNS_FILE_LOCATION'), 
	auth_key_id=os.getenv('KEY_ID'), 
	team_id=os.getenv('TEAM_ID')
)



def send_notification(text):
	print("[APNS] - sending tokens")
	payload = Payload(
		alert=text, 
		badge=1, 
		category="alert_preview", 
		mutable_content=True,
		url_args=["https://www.bhphotovideo.com/images/images2500x2500/sony_dsc_hx300_b_cyber_shot_dsc_hx300_digital_camera_926281.jpg"]
	)
	for i in ["1c98ea6d179feb7df23bf1c74324541e2c3a2aa1d0ac64b5d7c93c9490e0355d"]:
		try:
			client = APNsClient(credentials=token_credentials, use_sandbox=True)
			client.send_notification(i, payload, "com.rubensystems.home")
			
		except: 
			print(f"[ERROR] - sending notification to user")
			

	print("[APNS] - send notification")

send_notification("HI THERE")