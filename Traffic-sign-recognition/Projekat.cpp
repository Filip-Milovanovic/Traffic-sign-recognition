#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int y = 14;

Scalar color = (0, 0, 0);

int hmin = 0, smin = 0, vmin = 0;
int hmax = 179, smax = 255, vmax = 255;

Point topLeft, bottomRight;

Mat imgHSV, mask, imgCanny, imgDil, imgCrop, imgResized, matrix, imgWarp;




/////////////////////////////////  FUNKCIJA  /////////////////////////////////////////////////////
void getContours(Mat imgDil, Mat img)

{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;


	findContours(imgDil, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	//Filter za noise - za neke tackice i to..
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());
	string objectType;
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);
		cout << area << endl; //Printujemo areu

		if (area > 1000) {

			float peri = arcLength(contours[i], true);
			//Sad nalazimo krivine, npr ako ima 3 bice trougao itd...
			approxPolyDP(contours[i], conPoly[i], y, true);

			cout << conPoly[i].size() << endl;

			//Crtanje pravougaonika okolo
			boundRect[i] = boundingRect(conPoly[i]);

			int objCor = (int)conPoly[i].size(); //Vraca broj coskova

			if (objCor == 3) {
				objectType = "Opasnost";
			}
			if (objCor == 4) {
				objectType = "Obavestenje";
			}
			if (objCor == 8) {
				objectType = "Stop";
			}
			if (objCor > 8) {
				objectType = "Naredba";
			}

			drawContours(img, conPoly, i, color, FILLED);
			rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 3);
			putText(img, objectType, { boundRect[i].x + 5, boundRect[i].y + 25 }, FONT_HERSHEY_DUPLEX, 0.7, Scalar(255, 255, 255), 2);
			topLeft = boundRect[i].tl();
			bottomRight = boundRect[i].br();
		}
	}

}
///////////////////////////////////////////////////////////////////////////////////////

void main() {


	//Ulazna slika
	string path = "znakovi/stop.jpg";
	Mat img = imread(path);
	


	if (path == "znakovi/stop.jpg") {
		hmin = 0;
		hmax = 179;
		smin = 118;
		smax = 236;
		vmin = 94;
		vmax = 255;

		color = (0, 0, 255);
	}
	else if (path == "znakovi/opasnost.jpg") {
		hmin = 0;
		hmax = 19;
		smin = 170;
		smax = 255;
		vmin = 184;
		vmax = 255;

		resize(img, imgResized, Size(), 3, 3);
		img = imgResized;
		y = 25;

		color = (0, 255, 0);
	}
	else if (path == "znakovi/obavestenje.jpg") {
		hmin = 0;
		hmax = 136;
		smin = 113;
		smax = 255;
		vmin = 147;
		vmax = 255;

		resize(img, imgResized, Size(), 3, 3);
		img = imgResized;

		color = (255, 0, 0);
	}
	else if (path == "znakovi/naredba.jpg") {
		hmin = 0;
		hmax = 179;
		smin = 75;
		smax = 255;
		vmin = 172;
		vmax = 255;

		resize(img, imgResized, Size(), 3, 3);
		img = imgResized;
		y = 1;

		color = (120, 255, 66);

	}

	cvtColor(img, imgHSV, COLOR_BGR2HSV);

	/*namedWindow("Trackbars", (640, 200));
	createTrackbar("Hue Min", "Trackbars", &hmin, 179); 
	createTrackbar("Hue Max", "Trackbars", &hmax, 179);
	createTrackbar("Sat Min", "Trackbars", &smin, 255);
	createTrackbar("Sat Max", "Trackbars", &smax, 255);
	createTrackbar("Val Min", "Trackbars", &vmin, 255);
	createTrackbar("Val Max", "Trackbars", &vmax, 255); */

	//while (true) {


		Scalar lower(hmin, smin, vmin);
		Scalar upper(hmax, smax, vmax);
		
		//Pretvaranje u HSV
		inRange(imgHSV, lower, upper, mask);


		//Detekcija ivica pomocu Canny, onda podebljanje istih putem Dilate
		Canny(mask, imgCanny, 25, 75);
		Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
		dilate(imgCanny, imgDil, kernel);

		//Prepoznavanje znaka na osnovu oblika
		getContours(imgDil, img);

		//cout << "Top left: " << topLeft << endl;
		//cout << "Bottom right: " << bottomRight << endl;

		//Cropovanje oznacenog dela
		Rect roi(topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y); //Pravougaonik: x, y, sirina, visina
		imgCrop = img(roi);


		//Ispravljanje znakova ako su iskrivljeni
		float w = bottomRight.x - topLeft.x;
		float h = bottomRight.y - topLeft.y;

		Point2f src[4] = { {0,0}, {(float)bottomRight.x - topLeft.x,0}, {0,(float)bottomRight.y - topLeft.y}, {(float)bottomRight.x - topLeft.x,(float)bottomRight.y - topLeft.y} };
		Point2f dst[4] = { {0.0f,0.0f}, {w,0.0f}, {0.0f,h}, {w,h} };

		matrix = getPerspectiveTransform(src, dst);
		warpPerspective(imgCrop, imgWarp, matrix, Point(w, h));


		imshow("Original", img);
		//imshow("Mask", mask);
		imshow("Dilate", imgDil);
		//imshow("Cropped", imgCrop);
		imshow("Warped", imgWarp);

		//waitKey(1);
		waitKey(0);
	//}

}