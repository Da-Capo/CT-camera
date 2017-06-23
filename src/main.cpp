#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <ctype.h>

#include "Tracker.h"
using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
Point origin;
Rect selection;

static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
        break;
    }
}


int main (int argc, char* argv[])
{
    VideoCapture cap;
    Rect trackWindow;

    cap.open(0);

    if( !cap.isOpened() )
    {
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
    } 
    namedWindow( "Tracker", 0 );
    setMouseCallback( "Tracker", onMouse, 0 );
    
    Mat frame;
    Mat gray;
    double sum_time = 0.0;
    int count = 0;
    char fps[20];
    Tracker tracker;

    bool paused = false;

    for(;;)
    {
        if( !paused )
        {
            cap >> frame;
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);

        if( !paused )
        {
            
            // cv::Mat currentImage = image;
            cvtColor (image, gray, CV_RGB2GRAY);

            if( trackObject )
            {   

                if( trackObject < 0 )
                {   
                    trackWindow = selection;
                    std::cout << trackWindow.x << ", " << trackWindow.y << ", ";
                    std::cout << trackWindow.width  + trackWindow.x << ", "
                            << trackWindow.height + trackWindow.y << std::endl;
                    tracker.Init(gray, trackWindow);
                    sum_time = 0.0;
                    count = 0;
                    fps[20];
                }

                double t = static_cast <double> (cv::getTickCount ());
                trackWindow = tracker.TrackObject (gray);
                t = static_cast <double> (cv::getTickCount () - t) / cv::getTickFrequency();

                sum_time += t;

                if (sum_time >= 1.0)
                {
                    sprintf  (fps, "FPS: %d", count);

                    sum_time = .0;
                    count = 0;
                }

                tracker.DrawObject (image);
                cv::putText (image, fps, cv::Point (10, image.rows - 10),
                            cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 255));


                if( trackWindow.area() <= 1 )
                {
                    int cols = gray.cols; 
                    int rows = gray.rows; 
                    int r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }
            }
        }
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow( "Tracker", image );

        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
        case 'b':
            backprojMode = !backprojMode;
            break;
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    }

    return 0;


// cv::Rect objectPosition (
//         atoi( argv[ 2 ] ), // X coordinate
//         atoi( argv[ 3 ] ), // Y coordinate
//         atoi( argv[ 4 ] ) - atoi( argv[ 2 ] ), // width
//         atoi( argv[ 5 ] ) - atoi( argv[ 3 ] ) // height
//     );

//     ListOfFilenames imageFilenameList = loadListOfFilenames (argv[1]);

//     if (imageFilenameList.empty())
//     {
//         std::cerr << "Can't load image list from file " << argv[1] << std::endl;
//         return -1;
//     }

//     Tracker tracker;
//     cv::Mat startImage = cv::imread (imageFilenameList.front(), 0);

//     if (!startImage.data)
//     {
//         std::cerr << "Can't load first image" << std::endl;
//         return -1;
//     }

//     tracker.Init (startImage, objectPosition);
//     cv::Mat gray;
//     double sum_time = 0.0;
//     int count = 0;
//     char fps[20];

//     while (!imageFilenameList.empty())
//     {
//         cv::Mat currentImage = cv::imread (imageFilenameList.front());
//         imageFilenameList.pop_front();

//         if (currentImage.data)
//         {
//             cvtColor (currentImage, gray, CV_RGB2GRAY);

//             double t = static_cast <double> (cv::getTickCount ());
//             objectPosition = tracker.TrackObject (gray);
//             t = static_cast <double> (cv::getTickCount () - t) / cv::getTickFrequency();

//             sum_time += t;

//             if (sum_time >= 1.0)
//             {
//                 sprintf  (fps, "FPS: %d", count);

//                 sum_time = .0;
//                 count = 0;
//             }

//             count++;
//             std::cout << objectPosition.x << ", " << objectPosition.y << ", ";
//             std::cout << objectPosition.width  + objectPosition.x << ", "
//                       << objectPosition.height + objectPosition.y << std::endl;

//             tracker.DrawObject (currentImage);
//             cv::putText (currentImage, fps, cv::Point (10, currentImage.rows - 10),
//                          cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar (0, 255, 255));

//             imshow ("Tracker", currentImage);
//             if (cvWaitKey(2) == 'q') {	break; }
//         }
//     }

//     return 0;

}
