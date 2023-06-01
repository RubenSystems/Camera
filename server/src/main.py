from rsics_server import rsics_server
from ctypes import create_string_buffer, c_char, c_bool, memmove, string_at, c_uint8
from threading import Thread
import numpy as np
import cv2
from inference import Inference
from camera import Camera

server = rsics_server("5253")
inference = Inference()

clients = {}



def _new_image(ctx_p, from_p, type, ip_p, data_p, data_size):



	string_buffer = create_string_buffer(17)
	array_type = c_uint8 * data_size
	image_array = np.frombuffer(array_type.from_address(data_p), dtype=np.uint8)

	memmove(string_buffer, ip_p, 17)
	client_ip = string_buffer.value.decode('utf-8')
	print(client_ip)
	opencv_image = cv2.imdecode(image_array, cv2.IMREAD_COLOR)
	if client_ip not in clients: 
		clients[client_ip] = Camera(client_ip)
	res = inference.run(opencv_image, clients[client_ip].tracker)

	print(res)







if __name__ == "__main__":
	print("[SERVER] - initalised")
	thread = Thread(target = server.start, args = (_new_image,), daemon=True)
	thread.start()

	while True:
		pass
 

