import cv2

url = "http://192.168.7.4:80/stream"  # Ajuste a porta e caminho de stream conforme o sketch
cap = cv2.VideoCapture(url)

face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')

while True:
    ret, frame = cap.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.3, 5)

    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
        x_center = x + w//2
        y_center = y + h//2



    cv2.imshow("Face detection", frame)


    
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
