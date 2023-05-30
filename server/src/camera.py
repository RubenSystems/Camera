from object_tracker import ObjectTracker

class Camera:

	def __init__(self, uid):
		self.uid = uid
		self.tracker = ObjectTracker('sort').trackerObject.mot_tracker

