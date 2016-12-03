import numpy as np
import cv2
for k in dir(cv2):
  print k

cap = cv2.VideoCapture('test/my_video-5.mkv')
fgbg = cv2.createBackgroundSubtractorMOG2()

while(1):
  ret, frame = cap.read()
  fgmask = fgbg.apply(frame)
  masked = cv2.bitwise_and(frame, frame, fgmask)  
  cv2.imshow('frame', frame)
  k = cv2.waitKey(30) & 0xff
  if k == 27:
    break
 
cap.release()
cv2.destroyAllWindows()	
