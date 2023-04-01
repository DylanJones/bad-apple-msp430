#!/usr/bin/env python3
import cv2
import lzma
import zlib
import numpy as np
import wave

OUTPUT_SIZE = (180, 128)

### Bad Apple encoding script ###
def main():
    # Open the video file
    cap = cv2.VideoCapture("badapple.mp4")
    # Open the output badapple-encoded.mp4 file, using H.265 compression
    # out = cv2.VideoWriter("badapple-encoded.mp4", cv2.VideoWriter_fourcc(*"hev1"), 30, OUTPUT_SIZE)
    # Open the output badapple-encoded.wav file, using PCM 8-bit unsigned mono, 44100 Hz
    wav = wave.open("badapple-encoded.wav", 'rb')
    # open output binary file for writing
    binary_output = open("badapple-encoded.bin", "wb")
    frame_prev = None

    i = 0

    # Loop through the video
    while cap.isOpened():
        # Read the next frame
        ret, frame = cap.read()
        # If we're at the end of the video, stop
        if not ret:
            break

        # Convert the frame to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # Threshold frame to black and white
        ret, thresh = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)
        # Resize the frame to 80x45
        resized = cv2.resize(thresh, OUTPUT_SIZE)

        # Squeeze down to one bit per pixel
        squished = np.packbits(resized.T, axis=1)
        # Convert to a list of bytes
        squished = squished.tobytes()

        # Write the frame to the output file, converting back to BGR for compatibility
        # out.write(cv2.cvtColor(resized, cv2.COLOR_GRAY2BGR))
        # Write the compressed frame to the binary file
        binary_output.write(squished)
        # Grab 33ms of audio, then bitshift it down to fit w/ our 6-bit dac
        audio = wav.readframes(44100 // 30)
        audio = np.frombuffer(audio, dtype=np.uint8) >> 2
        # Write the audio to the binary file
        binary_output.write(audio.tobytes())

        # Print out progress bar and frame size
        print(f"Frame {i}, frame size: {len(squished)}, audio size {44100 // 30}, total frame size {len(squished) + 44100//30}", end="\r")
        i += 1

        # Save the current frame for the next frame
        frame_prev = squished

    print()
    # Close the video files
    cap.release()
    # out.release()
    binary_output.close()


if __name__ == "__main__":
    main()
