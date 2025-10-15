import face_recognition
import numpy as np
import os
import pathlib
import logging
import pyzed.sl as sl

class FRWorker():
    def __init__(self):
        self.logger = logging.getLogger("FRWorker")
        self.logger.setLevel(logging.DEBUG)

        self.face_locations = []
        self.face_encodings = []
        self.face_names = []

        self.known_face_encodings = []
        self.known_face_names = []

        self.logger.debug("Loading and encoding face images from 'faces' directory")

        base_dir = pathlib.Path(__file__).parent.absolute()
        faces_dir = os.path.join(base_dir, "faces")

        img_files = [f for f in os.listdir(faces_dir) if f.lower().endswith(('.jpg', '.jpeg', '.png'))]
        total_imgs = len(img_files)
        for idx, img_file in enumerate(img_files, 1):
            img_path = os.path.join(faces_dir, img_file)
            image = face_recognition.load_image_file(img_path)
            encodings = face_recognition.face_encodings(image)
            if encodings:
                self.known_face_encodings.append(encodings[0])
                # Extract name before the last '-' character
                name = os.path.splitext(img_file)[0].rsplit('-', 1)[0]
                self.known_face_names.append(name)
                percent = (idx / total_imgs) * 100 if total_imgs > 0 else 100
                self.logger.debug(f"Loaded and encoded {img_file} ({percent:.2f}%)")

        self.cv_image = None
        self.rgb_image = None
        self.is_running = True
    
    def destroy(self):
        self.is_running = False

    def faceid(self):
        pass
        # while self.is_running:
        #     if self.cv_image is None:
        #         continue
            
        #     rgb_image = self.cv_image.copy()
                        
        #     self.face_locations = face_recognition.face_locations(rgb_image, model="cnn")            
        #     self.face_encodings = face_recognition.face_encodings(rgb_image, self.face_locations)
        
        #     self.face_names = []

        #     for face_encoding in self.face_encodings:
        #         matches = face_recognition.compare_faces(self.known_face_encodings, face_encoding)
        #         name = "Visitor-en"

        #         face_distances = face_recognition.face_distance(self.known_face_encodings, face_encoding)
        #         best_match_index = np.argmin(face_distances)
                
        #         if matches[best_match_index] and face_distances[best_match_index] < 0.50: # Adjust threshold as needed
        #             name = self.known_face_names[best_match_index]
            