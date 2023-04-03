#!/usr/bin/env python3
import cv2
import lzma
import zlib
import numpy as np
import wave

OUTPUT_SIZE = (160, 128)

### Bad Apple encoding script ###
def main():
    # Open the video file
    cap = cv2.VideoCapture("lagtrain.mp4")
    # Open the output badapple-encoded.mp4 file, using H.265 compression
    out = cv2.VideoWriter("lagtrain-encoded.mp4", cv2.VideoWriter_fourcc(*"hev1"), 30, OUTPUT_SIZE)
    # Open the output badapple-encoded.wav file, using PCM 8-bit unsigned mono, 44100 Hz
    wav = wave.open("lagtrain-encoded.wav", 'rb')
    # open output binary file for writing
    binary_output = open("lagtrain-encoded.bin", "wb")
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
        out.write(cv2.cvtColor(resized, cv2.COLOR_GRAY2BGR))
        # Write the compressed frame to the binary file
        binary_output.write(squished)
        # Grab 33ms of audio, then bitshift it down to fit w/ our 6-bit dac
        audio = wav.readframes(44100 // 30)
        audio = np.frombuffer(audio, dtype=np.uint8) >> 2
        # Write the audio to the binary file
        binary_output.write(audio.tobytes())

        # This video is 15fps, so grab another audio frame and duplicate the video frame
        audio = wav.readframes(44100 // 30)
        audio = np.frombuffer(audio, dtype=np.uint8) >> 2
        binary_output.write(squished)
        binary_output.write(audio.tobytes())
        
        print(len(squished), len(audio.tobytes()), end=', ')

        # Print out progress bar and frame size
        print(f"Frame {i}, frame size: {len(squished)}, audio size {44100 // 30}, total frame size {len(squished) + 44100//30}", end="\r")
        i += 1

        # Save the current frame for the next frame
        frame_prev = squished

        # Display the frame (scaled up for display)
        cv2.imshow("frame", cv2.resize(resized, (640, 480), interpolation=cv2.INTER_NEAREST))
        # Wait for 1ms, or until a key is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    print()
    # Close the video files
    cap.release()
    out.release()
    binary_output.close()


if __name__ == "__main__":
    main()
