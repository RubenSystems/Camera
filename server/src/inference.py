import tflite_runtime.interpreter as tflite
import re
import numpy as np
from PIL import Image, ImageDraw
import imutils
import cv2

from pycoral.adapters.common import input_size
from pycoral.adapters.detect import get_objects
from pycoral.utils.dataset import read_label_file
from pycoral.utils.edgetpu import make_interpreter
from pycoral.utils.edgetpu import run_inference



class Inference:
	def __init__(self):
		self.interpreter = make_interpreter(
			"../models/ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite")
		self.interpreter.allocate_tensors()
		self.labels = self.__load_labels("../models/coco_labels.txt")
		self.inference_size = input_size(self.interpreter)

	def run(self, src_image, tracker):
		img = cv2.cvtColor(src_image, cv2.COLOR_BGR2RGB)
		img = cv2.resize(img, self.inference_size)

		run_inference(self.interpreter, img.tobytes())
		objs = get_objects(self.interpreter, 0.4)
		return self.__append_objs_to_img(
			src_image, self.inference_size, objs, self.labels, tracker)

	def __draw_text(self, img, text,
				 pos=(0, 0),
				 font_scale=3,
				 font_thickness=2,
				 padding=16,
				 text_color=(255, 255, 255),
				 text_color_bg=(149, 181, 0),
				 font=cv2.FONT_HERSHEY_SIMPLEX,
				 ):

		x, y = pos
		text_size, _ = cv2.getTextSize(text, font, font_scale, font_thickness)
		text_w, text_h = text_size
		img = cv2.rectangle(img, pos, (x +
								 text_w +
								 padding *
								 2, y +
								 text_h +
								 padding *
								 2), text_color_bg, -
					  1)
		img = cv2.putText(
			img,
			text,
			(x +
			 padding,
			 y +
			 text_h +
			 font_scale -
			 1 +
			 padding),
			font,
			font_scale,
			text_color,
			font_thickness)

		return img

	def __load_labels(self, path):
		p = re.compile(r'\s*(\d+)(.+)')
		with open(path, 'r', encoding='utf-8') as f:
			lines = (p.match(line).groups() for line in f.readlines())
			return {int(num): text.strip() for num, text in lines}

	def __append_objs_to_img(self, cv2_im, inference_size, objs, labels, tracker):
		height, width, channels = cv2_im.shape
		scale_x, scale_y = width / \
			inference_size[0], height / inference_size[1]

		detections = []
		for obj in objs:
			label = labels.get(obj.id, obj.id)
			if label != "person":
				continue
			detections.append(
				[
					obj.bbox.xmin,
					obj.bbox.ymin,
					obj.bbox.xmax,
					obj.bbox.ymax,
					obj.score
				]
			)
		detections = np.array(detections)

		if not detections.any():
			return []
			
		trdata = tracker.update()
		trackers = []
		if (np.array(trdata)).size:
			for td in trdata:

				x0, y0, x1, y1, track_id = td[0].item(), td[1].item(
				), td[2].item(), td[3].item(), td[4].item()
				trackers.append(track_id)


		
		return trackers


# src_image = cv2.imread("testface.jpg")


# x = Inference()
# tracker  = ObjectTracker("sort").trackerObject.mot_tracker
# peo = x.inference(src_image, tracker)
# print(peo)
