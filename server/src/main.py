from rsics_server import rsics_server
from ctypes import create_string_buffer, c_char, c_bool, memmove, string_at, c_uint8
from threading import Thread
import numpy as np
import cv2
from inference import Inference
from camera import Camera, CameraManager

server = rsics_server("5253")
inference = Inference()

clients = CameraManager()



def _new_image(ctx_p, from_p, type, ip_p, data_p, data_size):

	string_buffer = create_string_buffer(17)
	array_type = c_uint8 * data_size
	image_array = np.frombuffer(array_type.from_address(data_p), dtype=np.uint8)

	memmove(string_buffer, ip_p, 17)
	client_ip = string_buffer.value.decode('utf-8')
	opencv_image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
	
	client = clients.get(client_ip) 
	res = inference.run(opencv_image, client.tracker)
	print(res)








if __name__ == "__main__":
	print("[SERVER] - initalised")
	thread = Thread(target = server.start, args = (_new_image,), daemon=True)
	thread.start()

	while True:
		pass
 

