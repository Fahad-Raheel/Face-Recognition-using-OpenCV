#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <opencv2/objdetect.hpp>

using namespace std;
using namespace cv;
using namespace cv::face;

// ── Constants ─────────────────────────────
const int MAX_USERS=20, MAX_FILES=100, MAX_LOG=300, SAMPLES=30, CONF_LIMIT=80;

// ── Helpers ───────────────────────────────
void printLine(char c='-', int n=52) {
    for(int i=0;i<n;i++) cout<<c;
    cout<<"\n";
}

string getString(const string& m) {
    string s;
    cout<<m;
    getline(cin,s);
    return s;
}

string getTime() {
    time_t t=time(nullptr);
    char b[64];
    strftime(b,sizeof(b),"%Y-%m-%d %H:%M:%S",localtime(&t));
    return b;
}

int getInt(const string& m) {
    int v;
    while(true){
        cout<<m;
        if(cin>>v){cin.ignore();return v;}
        cin.clear();
        cin.ignore(1000,'\n');
        cout<<"Enter a number.\n";
    }
}