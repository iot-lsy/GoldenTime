import cv2
import timeit

# 영상 정보 불러오기
video = cv2.VideoCapture('FALL-Backwards_.mp4')
# 가우시안 혼합 배경제거 알고리즘
fgbg = cv2.createBackgroundSubtractorMOG2(history=0, varThreshold=137, detectShadows=0)

diff_x = [0, 0]
diff_y = [0, 0]
rect_count = 0
fall_count = 0


def MOG(frame):
    global rect_count
    global fall_count

    fgmask = fgbg.apply(frame)
    results = cv2.findContours(
        fgmask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

    for contour in results[0]:
        x, y, w, h = cv2.boundingRect(contour)
        if w*h >= 18000:
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
            rect_count += 1

    if rect_count == 1:
        A = ((x + w) + y) / 2
        B = (x + (y + h)) / 2

        if A > B :
            fall_count += 1

    cv2.imshow('fgmask', fgmask)


while True:

    ret, frame = video.read()

    if ret is True:

        #width = frame.shape[1]
        #height = frame.shape[0]
        #frame = cv2.resize(frame, (int(width * 0.2), int(height * 0.2)))

        MOG(frame)

        cv2.imshow('video', frame)

        print(rect_count)
        rect_count = 0
        #all_count = 0

        if cv2.waitKey(1) > 0:
            break