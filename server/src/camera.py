from object_tracker import ObjectTracker

class Camera:

	def __init__(self, uid):
		self.uid = uid
		self.tracker = ObjectTracker('sort').trackerObject.mot_tracker

class CameraManager:

	def __init__(self):
		self.cameras = {}

	def get(self, uid):
		if uid not in self.cameras:
			self.cameras[uid] = Camera(uid)

		return self.cameras[uid]