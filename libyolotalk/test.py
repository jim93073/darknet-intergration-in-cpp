import signal
import sys

from libyolotalk import BoundingBox
from libyolotalk import Mat
from libyolotalk import YoloDevice

device = YoloDevice(
    cfg="./weights/yolov4.cfg",
    weights="./weights/yolov4.weights",
    name_list="./weights/coco.names",
    url="rtsp://iottalk:iottalk2019@140.113.237.220:554/live2.sdp",
    show_msg=True,
    # polygon=[(10, 20), (30, 40), (50, 60)],
)

def listener(frame_id, mat, bboxes, file_path):
    print(frame_id)
    print(mat.getData())
    for b in bboxes:
        print("\t", b.get_class_id(), b.get_name(), b.get_confidence())
    print(file_path, "\n")

device.setPredictionListener(listener)

poly = device.getPolygon()
for p in poly:
    print(p.x, p.y)
print(len(poly))

device.start()

running = True

def signal_handler(sig, frame):
    global running
    running = False
    print('You pressed Ctrl+C!')
    device.stop(force=True)

signal.signal(signal.SIGINT, signal_handler)
print('Press Ctrl+C')

while running:
    pass

print("COLOR", device.getColors(class_id=5))
print("FPS", device.getFps())
print("Model FPS", device.getModelFps())
print("Video FPS", device.getVideoFps())

device.join()
