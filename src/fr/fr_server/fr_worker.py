import face_recognition
import numpy as np
import os
import pathlib
import logging
import pyzed.sl as sl
import time
import cv2
import random

class FRWorker():
    def __init__(self):
        # worker parameters
        self.timeout = 3  # seconds

        # logging setup
        self.logger = logging.getLogger("FRWorker")
        self.logger.setLevel(logging.DEBUG)

        # zed camera setup
        self.zed : sl.Camera = sl.Camera()
        self.init : sl.InitParameters = sl.InitParameters()
        self.init.set_from_stream("127.0.0.1", 30000) # Connect to ZED camera stream

        # face recognition setup
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

    def faceid(self) -> tuple[str, str, str]:
        """
        Perform face recognition using the ZED camera to identify visitors.
        This method captures images from the ZED camera, detects faces, and attempts to match them
        against known face encodings. It requires consistent detection across multiple frames to
        confirm an identification, reducing false positives.
        Args:
            None
        Returns:
            tuple[str, str, str]: A tuple containing:
                - random_id (str): A randomly generated visitor ID in format "srdXXXX" where XXXX is a zero-padded number (1-301)
                - name (str): The identified person's name, or "Visitor" if no match found or timeout occurs
                - language (str): The preferred language code (e.g., "en"), parsed from the known face name, or "en" as default
        Raises:
            None: Errors are logged but not raised; returns default values on failure
        Notes:
            - Displays a live preview window showing detected faces with bounding boxes and confidence scores
            - Requires 3 consistent frames of the same person before confirming identification
            - Uses a confidence threshold of 0.50 for face matching
            - Operates at ~10 FPS with a configurable timeout period
            - Automatically closes the ZED camera and cleanup resources in finally block
        """
        # Generate a random ID
        random_id = f"srd{random.randint(1, 301):04d}"
        
        # Filter variables to reduce impact of different recognition results
        last_identified_name = None
        consistent_frames = 0
        confidence_threshold = 0.50
        consistency_requirement = 3  # Need 3 consistent frames before accepting

        err : sl.ERROR_CODE = self.zed.open(self.init)
        if err != sl.ERROR_CODE.SUCCESS:
            self.logger.error(f"Failed to open ZED camera: {err}")
            return random_id, "Visitor", "en"
        
        try:
            self.logger.debug("[1] Starting image capture from ZED camera")

            # Create window for display
            window_name = "ZED Camera Capture"
            cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
            self.logger.debug("[2] Created display window")
            
            start_time : float = time.time()    

            while time.time() - start_time < self.timeout:
                self.logger.debug("[3] Attempting to grab frame from camera")
                # try to grab an image
                if self.zed.grab() == sl.ERROR_CODE.SUCCESS:
                    self.logger.debug("[4] Successfully grabbed frame")
                    mat : sl.Mat = sl.Mat()
                    self.zed.retrieve_image(mat, sl.VIEW.LEFT)
                    cv_image = mat.get_data()
                    
                    # Resize to 480p for faster processing
                    cv_image = cv2.resize(cv_image, (640, 480))
                    
                    # Verify the image
                    if cv_image is None:
                        self.logger.debug("[5] Image is None, skipping")
                        time.sleep(0.1)
                        continue

                    self.logger.debug("[6] Image captured from ZED camera")
                    rgb_image = cv2.cvtColor(cv_image, cv2.COLOR_BGRA2RGB)
                    self.logger.debug("[7] Converting to RGB and detecting faces")
                    face_locations = face_recognition.face_locations(rgb_image, model="hog")            
                    face_encodings = face_recognition.face_encodings(rgb_image, face_locations)

                    if not face_locations:
                        self.logger.debug("[8] No faces detected, continuing")
                        time.sleep(0.1)
                        continue

                    # Find the best match across all detected faces
                    best_name = "Visitor"
                    best_language = "en"
                    best_distance = float('inf')

                    self.logger.debug(f"[9] Detected {len(face_locations)} face(s) in the image")
                    for face_encoding in face_encodings:
                        face_distances = face_recognition.face_distance(self.known_face_encodings, face_encoding)
                        
                        if len(face_distances) > 0:
                            best_match_index = np.argmin(face_distances)
                            distance = face_distances[best_match_index]
                            
                            # Only consider if better than current best and below threshold
                            if distance < best_distance and distance < confidence_threshold:
                                best_distance = distance
                                matches = face_recognition.compare_faces(self.known_face_encodings, face_encoding)
                                
                                if matches[best_match_index]:
                                    full_name : str = self.known_face_names[best_match_index]
                                    # Parse name and language (e.g., "Rico-en" -> "Rico", "en")
                                    parts = full_name.rsplit('-', 1)
                                    best_name = parts[0] if len(parts) > 0 else "Visitor"
                                    best_language = parts[1] if len(parts) > 1 else "en"
                    
                    self.logger.debug("[10] Rendering display with bounding boxes")
                    # Draw bounding boxes and labels on the image
                    display_image = cv_image.copy()
                    for (top, right, bottom, left) in face_locations:
                        # Draw rectangle around face
                        cv2.rectangle(display_image, (left, top), (right, bottom), (0, 255, 0), 2)
                        
                        # Draw label with name and confidence
                        label = f"{best_name} ({best_distance:.2f})"
                        cv2.rectangle(display_image, (left, bottom - 25), (right, bottom), (0, 255, 0), cv2.FILLED)
                        cv2.putText(display_image, label, (left + 6, bottom - 6), 
                                   cv2.FONT_HERSHEY_DUPLEX, 0.6, (255, 255, 255), 1)
                    
                    cv2.imshow(window_name, display_image)
                    cv2.waitKey(1)

                    self.logger.debug("[11] Displayed image with detected faces")
                    self.logger.debug(f"[12] Best match: {best_name} with distance {best_distance:.2f}")
                    
                    # Apply consistency filter
                    if best_name == last_identified_name:
                        consistent_frames += 1
                        self.logger.debug(f"[13] Consistent frame count: {consistent_frames}/{consistency_requirement}")
                        if consistent_frames >= consistency_requirement:
                            # Confirmed identification
                            self.logger.debug(f"[14] Confirmed identification: {best_name}")
                            return random_id, best_name, best_language
                    else:
                        # Reset consistency counter
                        self.logger.debug(f"[15] Name changed from {last_identified_name} to {best_name}, resetting counter")
                        last_identified_name = best_name
                        consistent_frames = 1
                    
                time.sleep(0.1) # Sleep for ~1/10 seconds (10 FPS), stream is 60 FPS, but we only need 10 FPS

            self.logger.debug("[16] Timeout reached, returning default values")

            # Create window for display
            window_name = "ZED Camera Capture"
            cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
            
            start_time : float = time.time()    

            while time.time() - start_time < self.timeout:
                # try to grab an image
                if self.zed.grab() == sl.ERROR_CODE.SUCCESS:
                    mat : sl.Mat = sl.Mat()
                    self.zed.retrieve_image(mat, sl.VIEW.LEFT)
                    cv_image = mat.get_data()
                    
                    # Verify the image
                    if cv_image is None:
                        time.sleep(0.1)
                        continue

                    self.logger.debug("Image captured from ZED camera")
                    rgb_image = cv2.cvtColor(cv_image, cv2.COLOR_BGRA2RGB)
                    face_locations = face_recognition.face_locations(rgb_image, model="cnn")            
                    face_encodings = face_recognition.face_encodings(rgb_image, face_locations)

                    if not face_locations:
                        time.sleep(0.1)
                        continue

                    # Find the best match across all detected faces
                    best_name = "Visitor"
                    best_language = "en"
                    best_distance = float('inf')

                    self.logger.debug(f"Detected {len(face_locations)} face(s) in the image")
                    for face_encoding in face_encodings:
                        face_distances = face_recognition.face_distance(self.known_face_encodings, face_encoding)
                        
                        if len(face_distances) > 0:
                            best_match_index = np.argmin(face_distances)
                            distance = face_distances[best_match_index]
                            
                            # Only consider if better than current best and below threshold
                            if distance < best_distance and distance < confidence_threshold:
                                best_distance = distance
                                matches = face_recognition.compare_faces(self.known_face_encodings, face_encoding)
                                
                                if matches[best_match_index]:
                                    full_name : str = self.known_face_names[best_match_index]
                                    # Parse name and language (e.g., "Rico-en" -> "Rico", "en")
                                    parts = full_name.rsplit('-', 1)
                                    best_name = parts[0] if len(parts) > 0 else "Visitor"
                                    best_language = parts[1] if len(parts) > 1 else "en"
                    
                    # Draw bounding boxes and labels on the image
                    display_image = cv_image.copy()
                    for (top, right, bottom, left) in face_locations:
                        # Draw rectangle around face
                        cv2.rectangle(display_image, (left, top), (right, bottom), (0, 255, 0), 2)
                        
                        # Draw label with name and confidence
                        label = f"{best_name} ({best_distance:.2f})"
                        cv2.rectangle(display_image, (left, bottom - 25), (right, bottom), (0, 255, 0), cv2.FILLED)
                        cv2.putText(display_image, label, (left + 6, bottom - 6), 
                                   cv2.FONT_HERSHEY_DUPLEX, 0.6, (255, 255, 255), 1)
                    
                    cv2.imshow(window_name, display_image)
                    cv2.waitKey(1)

                    self.logger.debug("DIsplayed image with detected faces")
                    self.logger.debug(f"Best match: {best_name} with distance {best_distance:.2f}")
                    
                    # Apply consistency filter
                    if best_name == last_identified_name:
                        consistent_frames += 1
                        if consistent_frames >= consistency_requirement:
                            # Confirmed identification
                            return random_id, best_name, best_language
                    else:
                        # Reset consistency counter
                        last_identified_name = best_name
                        consistent_frames = 1
                    
                time.sleep(0.1) # Sleep for ~1/10 seconds (10 FPS), stream is 60 FPS, but we only need 10 FPS

            # If loop exits without identification, return default
            return random_id, "Visitor", "en"
            
        finally:
            # Cleanup resources guaranteed to execute
            cv2.destroyWindow(window_name)
            self.logger.debug("Closing ZED camera")
            self.zed.close()