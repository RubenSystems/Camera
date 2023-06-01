from ctypes import *

rsics = CDLL('../lib/librsics2.so')

callback_func_type = CFUNCTYPE(None, c_void_p, c_void_p, c_int, c_char_p, c_void_p, c_ulong)


# def _new_image(ctx_p, from_p, type, data_p, data_size):




class rsics_server: 

	def __init__(self, port):

		self._computer = rsics.rsics_init_connection() 
		self._callback = None
		rsics.rsics_make_listener(bytes(str(port), "utf-8"), self._computer)

	
	def start(self, recv_callback):
		_callback = callback_func_type(recv_callback)

		self._callback = rsics.alloc_message_callback(self._computer, _callback)
		self._listening = c_bool(True)

		rsics.rsics_listen(self._computer, pointer(self._listening), self._callback)

	def stop(self):
		self._listening = c_bool(False)
		if self._callback is not None:
			rsics.delete_message_callback(self._callback)
			self._callback = None

	def __del__(self):
		self.stop()
		rsics.rsics_free_connection(self._computer)

