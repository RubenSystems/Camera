from ctypes import *

rsics = CDLL('../lib/librsics2.so')

callback_func_type = CFUNCTYPE(None, c_void_p, c_void_p, c_int, c_void_p, c_ulong)

# rsics.alloc_message_callback.argtypes = [c_void_p, callback_func_type]
# rsics.alloc_message_callback.restype = c_void_p

# rsics.rsics_listen.argtypes = [c_void_p, c_void_p, c_void_p]
# rsics.rsics_listen.restype = None


def _new_image(ctx_p, from_p, type, data_p, data_size):
	print("HI THERE!!!")

_callback = callback_func_type(_new_image)

class rsics_server: 

	def __init__(self, port: int):
		self._computer = rsics.rsics_init_connection() 
		rsics.rsics_make_listener(bytes(str(port), "utf-8"), self._computer)

		self._callback = rsics.alloc_message_callback(self._computer, _callback)
		self._listening = c_bool(True)
		print("botajef")
		rsics.rsics_listen(self._computer, pointer(self._listening), self._callback)


	def __del__(self):
		rsics.rsics_free_connection(self._computer)
		rsics.delete_message_callback(self._callback)

x = rsics_server("5253")
