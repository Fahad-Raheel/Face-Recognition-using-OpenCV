#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>

using namespace std;
using namespace cv;
const int SAMPLES = 30;

class User {
protected:
    int userId;
    int permLevel;
    int loginCount;
    string username;
    string fullName;
    string lastLogin;
    bool faceReady;

public:
    User() {
        userId = 0;
        permLevel = 1;
        loginCount = 0;
        username = "none";
        fullName = "none";
        lastLogin = "Never";
        faceReady = false;
    }

    User(int id, string un, string fn, int perm) {
        userId = id;
        username = un;
        fullName = fn;
        permLevel = perm;
        loginCount = 0;
        lastLogin = "Never";
        faceReady = false;
    }

    virtual void showInfo() {
        cout << "\n--- USER INFO ---\n";
        cout << "ID: " << userId << "\n";
        cout << "Username: " << username << "\n";
        cout << "Full Name: " << fullName << "\n";
        cout << "Permission: " << permLevel << "\n";
        cout << "Face Ready: " << faceReady << "\n";
    }

    int getId() { return userId; }
    string getName() { return fullName; }
};

class AdminUser : public User {
private:
    string adminPass;

public:
    AdminUser(int id, string un, string fn, string pass)
        : User(id, un, fn, 3) {
        adminPass = pass;
    }

    bool checkPassword(string pass) {
        return pass == adminPass;
    }

    void onLogin() {
        cout << "ADMIN LOGIN SUCCESS\n";
    }
};

class VaultFile {
private:
    int fileId;
    int requiredPerm;
    string filename;
    string owner;
    bool locked;

public:
    VaultFile(int id, string name, string own, int perm) {
        fileId = id;
        filename = name;
        owner = own;
        requiredPerm = perm;
        locked = true;
    }

    bool tryUnlock(int userPerm) {
        if (userPerm >= requiredPerm) {
            locked = false;
            return true;
        }
        return false;
    }

    void show() {
        cout << "\nFile: " << filename
             << " | Owner: " << owner
             << " | Status: " << (locked ? "LOCKED" : "OPEN") << endl;
    }
};

class CameraController {
private:
    VideoCapture camera;
    bool opened = false;

    CameraController() {}

public:
    static CameraController* instance;

    static CameraController* getInstance() {
        if (!instance)
            instance = new CameraController();
        return instance;
    }

    bool open(int index = 0) {
        if (opened) return true;

        camera.open(index);
        opened = camera.isOpened();

        if (opened) cout << "[Camera] Opened\n";
        else cout << "[Camera] Failed\n";

        return opened;
    }

    bool getFrame(Mat& frame) {
        if (!opened) return false;
        camera >> frame;
        return !frame.empty();
    }

    void close() {
        if (opened) camera.release();
        opened = false;
    }
};

CameraController* CameraController::instance = nullptr;
class Authenticator {
public:
    virtual bool registerFace(int userId, string username) = 0;
    virtual void recognizeFace() = 0;

    virtual bool saveModel(string path) = 0;
    virtual bool loadModel(string path) = 0;

    virtual bool isTrained() = 0;

    virtual string getModelName() = 0;   // MUST exist

    virtual ~Authenticator() {}
};

class FaceAI : public Authenticator {

private:
    Ptr<cv::face::LBPHFaceRecognizer> model;
    CascadeClassifier detector;

    string modelPath;
    bool trained;

    map<int, string> idToName;

public:

    FaceAI(string cascadePath, string mdl = "face_model.yml")
        : modelPath(mdl), trained(false)
    {
        model = cv::face::LBPHFaceRecognizer::create();

        if (!detector.load(cascadePath)) {
            cout << "❌ Cascade load failed\n";
        } else {
            cout << "✓ Cascade loaded\n";
        }
    }

    // ================= REGISTER =================
    bool registerFace(int userId, string username) override {

        cout << "🔥 Registering: " << username << endl;

        CameraController* cam = CameraController::getInstance();
        cam->open();

        vector<Mat> images;
        vector<int> labels;

        Mat frame, gray;
        int captured = 0;

        namedWindow("Register", WINDOW_AUTOSIZE);

        while (captured < 30) {

            if (!cam->getFrame(frame)) break;

            cvtColor(frame, gray, COLOR_BGR2GRAY);
            equalizeHist(gray, gray);

            vector<Rect> faces;
            detector.detectMultiScale(gray, faces, 1.1, 3, 0, Size(50,50));

            for (auto &f : faces) {

                Mat faceROI;
                resize(gray(f), faceROI, Size(200,200));

                images.push_back(faceROI);
                labels.push_back(userId);

                captured++;

                rectangle(frame, f, Scalar(0,255,0), 2);

                putText(frame,
                    "Capturing: " + to_string(captured),
                    Point(10,30),
                    FONT_HERSHEY_SIMPLEX,
                    0.8,
                    Scalar(0,255,0),
                    2
                );
            }

            imshow("Register", frame);

            if (waitKey(30) == 27) break;
        }

        destroyWindow("Register");

        if (captured < 10) {
            cout << "❌ Not enough samples\n";
            return false;
        }

        model->train(images, labels);
        model->save(modelPath);

        idToName[userId] = username;
        trained = true;

        cout << "✅ Face saved for: " << username << endl;

        return true;
    }

    // ================= RECOGNITION =================
    void recognizeFace() override {

        if (!trained) {
            model->read(modelPath);
            trained = true;
        }

        CameraController* cam = CameraController::getInstance();
        cam->open();

        Mat frame, gray;

        namedWindow("Recognition", WINDOW_AUTOSIZE);

        while (true) {

            if (!cam->getFrame(frame)) break;

            cvtColor(frame, gray, COLOR_BGR2GRAY);
            equalizeHist(gray, gray);

            vector<Rect> faces;
            detector.detectMultiScale(gray, faces, 1.1, 3);

            for (auto &f : faces) {

                Mat faceROI;
                resize(gray(f), faceROI, Size(200,200));

                int label;
                double conf;

                model->predict(faceROI, label, conf);

                string text;
                Scalar color;

                // ================= FIXED LOGIC =================
                if (conf > 55 || idToName.find(label) == idToName.end()) {
                    text = "UNKNOWN";
                    color = Scalar(0,0,255);
                } else {
                    text = idToName[label];
                    color = Scalar(0,255,0);
                }

                rectangle(frame, f, color, 2);

                putText(frame,
                    text,
                    Point(f.x, f.y - 10),
                    FONT_HERSHEY_SIMPLEX,
                    0.7,
                    color,
                    2
                );
            }

            imshow("Recognition", frame);

            if (waitKey(30) == 27) break;
        }

        destroyWindow("Recognition");
    }

    // ================= REQUIRED INTERFACE FUNCTIONS =================

    bool saveModel(string path) override {
        model->save(path);
        return true;
    }

    bool loadModel(string path) override {
        model->read(path);
        trained = true;
        return true;
    }

    bool isTrained() override {
        return trained;
    }

    string getModelName() override {
        return "LBPH Face Recognizer";
    }
};
int main() {

    cout << "=== FACE AI SYSTEM ===\n";

    FaceAI ai("haarcascade_frontalface_default.xml");

    int choice;

    while (true) {

        cout << "\n1. Register Face\n2. Recognize Face\n3. Exit\nChoice: ";
        cin >> choice;

        if (choice == 1) {

            int id;
            string name;

            cout << "Enter ID: ";
            cin >> id;

            cout << "Enter Name: ";
            cin >> name;

            ai.registerFace(id, name);

        } 
        else if (choice == 2) {

            ai.recognizeFace();

        } 
        else if (choice == 3) {

            break;

        } 
        else {
            cout << "Invalid choice\n";
        }
    }

    return 0;
}