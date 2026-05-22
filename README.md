This is a security system that uses C++ and OpenCV to recognize faces. It is based on the LBPH algorithm.
The Face Recognition System supports people registering their faces recognizing faces in time and checking if users are who they say they are.

- You can register your face using your webcam
- The system can recognize faces in time
- It uses the LBPH face recognition model
- It uses OpenCV Haar Cascade to detect faces
- It maps user IDs to names
- It can detect faces it does not know

- C++ is used to write the code
- OpenCV is used for face recognition
- The OpenCV Face Module with LBPH is used for face recognition
- CMake is used to configure the project
- vcpkg is used to install OpenCV

1. Get a copy of the repository
2. Install OpenCV using vcpkg
3. Use CMake to set up the project
4. Build the project
5. Run the file

The file `haarcascade_frontalface_default.xml` needs to be in the same directory as the executable file.
