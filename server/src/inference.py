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
		self.interpreter = make_interpreter("./models/ssd_mobilenet_v2_coco_quant_postprocess_edgetpu.tflite")
		self.interpreter.allocate_tensors()
		self.labels = __load_labels("./models/coco_labels.txt")
		self.inference_size = input_size(interpreter)

	def inference(self, src_image):
		img = cv2.cvtColor(src_image, cv2.COLOR_BGR2RGB)
		img = cv2.resize(img, inference_size)

		run_inference(interpreter, img.tobytes())
		objs = get_objects(interpreter, 0.4)
		return __append_objs_to_img(src_image, inference_size, objs, labels)



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
		img = cv2.rectangle(img, pos, (x + text_w + padding * 2, y + text_h + padding * 2), text_color_bg, -1)
		img = cv2.putText(img, text, (x + padding, y + text_h + font_scale - 1 + padding), font, font_scale, text_color, font_thickness)

		return img


	def __load_labels(self, path):
		p = re.compile(r'\s*(\d+)(.+)')
		with open(path, 'r', encoding='utf-8') as f:
			lines = (p.match(line).groups() for line in f.readlines())
			return {int(num): text.strip() for num, text in lines}


	def __append_objs_to_img(self, cv2_im, inference_size, objs, labels):
		height, width, channels = cv2_im.shape
		scale_x, scale_y = width / inference_size[0], height / inference_size[1]

		number_of_people = 0
		for obj in objs:
			label = labels.get(obj.id, obj.id)
			if label != "person":
				continue

			number_of_people += 1
			bbox = obj.bbox.scale(scale_x, scale_y)
			x0, y0 = int(bbox.xmin), int(bbox.ymin)
			x1, y1 = int(bbox.xmax), int(bbox.ymax)

			percent = int(100 * obj.score)
			label = '{}% {}'.format(percent, label)

			cv2_im = cv2.rectangle(cv2_im, (x0, y0), (x1, y1), (149, 181, 0), 8)
			cv2_im = draw_text(cv2_im, label, (x0, y0 - 50), )

		return cv2_im, number_of_people





# src_image = cv2.imread("testface.jpg")



# cv2.imwrite("img1.jpg", src_image)

