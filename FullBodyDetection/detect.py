import cv2
import mediapipe as mp
import numpy as np

mp_drawing = mp.solutions.drawing_utils
mp_pose = mp.solutions.pose

# For webcam input:
cap = cv2.VideoCapture('FALL-Backwards_.mp4') #webcam 쓸땐 0

with mp_pose.Pose(
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5) as pose:

  while cap.isOpened():
    success, image = cap.read()
    if not success:
      print("Ignoring empty camera frame.")
      # If loading a video, use 'break' instead of 'continue'.
      continue

    # Flip the image horizontally for a later selfie-view display, and convert
    # the BGR image to RGB.
    image = cv2.cvtColor(cv2.flip(image, 1), cv2.COLOR_BGR2RGB)

    # To improve performance, optionally mark the image as not writeable to
    # pass by reference.
    image.flags.writeable = False
    results = pose.process(image)

    # Draw the pose annotation on the image.
    image.flags.writeable = True
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

    
    mp_drawing.draw_landmarks(
        image, results.pose_landmarks, mp_pose.POSE_CONNECTIONS)
    cv2.imshow('MediaPipe Pose', image)

    def calculate(a,b,c):
      a = np.array(a)
      b = np.array(b)
      c = np.array(c)

      d1 = abs(a[0]-b[0])
      d2 = abs(a[0]-c[0])

      if d1>100 or d2>100:
          length = 0
      else :
          length = 1

      return length

    try :
      landmarks = results.pose_landmarks.landmark
      nose = [landmarks[mp_pose.PoseLandmark.NOSE.value].x,landmarks[mp_pose.PoseLandmark.NOSE.value].y]
      left_foot = [landmarks[mp_pose.PoseLandmark.LEFT_FOOT_INDEX.value].x,landmarks[mp_pose.PoseLandmark.LEFT_FOOT_INDEX.value].y]
      right_foot = [landmarks[mp_pose.PoseLandmark.RIGHT_FOOT_INDEX.value].x,landmarks[mp_pose.PoseLandmark.RIGHT_FOOT_INDEX.value].y]

      length = calculate(nose, left_foot, right_foot)

      if length == 0:
            cv2.putText(image, text='State:dangerous')
      else:
            cv2.putText(image, text='State:safety')        
    except:
      pass

    if cv2.waitKey(5) & 0xFF == 27:
      break

  cap.release()


