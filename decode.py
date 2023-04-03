#!/usr/bin/env python3
import cv2
import lzma
import zlib
import numpy as np
import wave

OUTPUT_SIZE = (160, 128)
AUDIO_SIZE = 44100 // 30
VIDEO_SIZE = 160 * 128 // 8
FRAME_SIZE = AUDIO_SIZE + VIDEO_SIZE

### Verify correct encoding by decoding the file
def main():
    i = 0
    # Open the encoded file
    with open("lagtrain-encoded.bin", "rb") as f:
        while True:
            # Frame container
            frame = np.zeros(OUTPUT_SIZE, dtype=np.uint8)
            for r in range(OUTPUT_SIZE[0]):
                # Read a single row
                row = f.read(OUTPUT_SIZE[1] // 8)
                # Convert to np bitarray
                row = np.unpackbits(np.frombuffer(row, dtype=np.uint8))
                # Add to frame
                frame[r] = row
            # Read audio
            audio = f.read(AUDIO_SIZE)
            # Discard.

            # Get frame back to full brightness
            frame *= 255
            # Display!
            cv2.imshow("frame", frame)
            # Wait for keypress
            k = cv2.waitKey(33)
            # Quit on q
            if k == ord("q"):
                break
#             frame = f.read(FRAME_SIZE)
#             video_portion = frame[:VIDEO_SIZE]
#             audio_portion = frame[VIDEO_SIZE:]
# 
#             # Decompress the video back into pixels
#             # First, reshape into its packed-by-line form
#             video = np.frombuffer(video_portion, dtype=np.uint8).reshape(128 // 8, 180)
#             # Then, unpack the bits
#             video = np.unpackbits(video, axis=1)
#             # Stretch back to 255
#             video = video * 255
#             # Finally, reshape into the proper format
#             video = video.reshape(OUTPUT_SIZE)
# 
#             # Display the video
#             cv2.imshow("video", video)
#             k = cv2.waitKey(33)
#             if k == ord("q"):
#                 break
# 
#             # Print progress
#             i += 1
#             print(f"Frame {i}")


if __name__ == "__main__":
    main()
