#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>

wchar_t* projectPath;

Rect selectedRect;
bool drawing = false;
bool rectReady = false;
Point startPoint;

void mouseHandlerRedEye(int event, int x, int y, int flags, void* param)
{
    Mat* img = (Mat*)param;

    if (event == EVENT_MOUSEMOVE && drawing) // while drawing
    {
        selectedRect = Rect(min(startPoint.x, x), min(startPoint.y, y), abs(x - startPoint.x), abs(y - startPoint.y));
        Mat temp = img->clone();
        rectangle(temp, selectedRect, Scalar(0, 255, 0), 2);
        imshow("Original Image", temp);
    }
    else if (event == EVENT_LBUTTONDOWN)
    {
        drawing = true;
        startPoint = Point(x, y);
        selectedRect = Rect(x, y, 0, 0);
    }
    else if (event == EVENT_LBUTTONUP) // button released
    {
        drawing = false;
        rectReady = true; // set flag!
        selectedRect = Rect(min(startPoint.x, x), min(startPoint.y, y), abs(x - startPoint.x), abs(y - startPoint.y));
        Mat temp = img->clone();
        rectangle(temp, selectedRect, Scalar(0, 255, 0), 2);
        imshow("Original Image", temp);
    }
}

void redEyeRemovalRGBThresholding()
{
    char fname[MAX_PATH];
    while (openFileDlg(fname))
    {
        Mat src = imread(fname, IMREAD_COLOR);
        if (src.empty())
        {
            printf("Could not open image\n");
            continue;
        }

        Mat dst = src.clone();
        rectReady = false;
        selectedRect = Rect();

        namedWindow("Original Image");
        setMouseCallback("Original Image", mouseHandlerRedEye, &src);

        printf("Draw a rectangle around the eye region using your mouse.\n");

        while (!rectReady) // while flag not set
        {
            imshow("Original Image", src);
            if (waitKey(30) == 27)
                break;
        }

        if (selectedRect.width > 0 && selectedRect.height > 0)
        {
            selectedRect &= Rect(0, 0, src.cols, src.rows);
            for (int i = selectedRect.y; i < selectedRect.y + selectedRect.height; i++)
            {
                for (int j = selectedRect.x; j < selectedRect.x + selectedRect.width; j++)
                {
                    Vec3b pixel = src.at<Vec3b>(i, j);
                    uchar b = pixel[0];
                    uchar g = pixel[1];
                    uchar r = pixel[2];

                    if (r > 100 && r > g * 1.5 && r > b * 1.5)
                    {
                        dst.at<Vec3b>(i, j)[0] = (g + b) / 2;
                        dst.at<Vec3b>(i, j)[1] = (g + b) / 2;
                        dst.at<Vec3b>(i, j)[2] = (g + b) / 2;
                    }
                }
            }
        }

        imshow("Original Image", src);
        imshow("Red-Eye Corrected Image", dst);

        waitKey();
        destroyAllWindows();
    }
}

int main()
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
    projectPath = _wgetcwd(0, 0);
    redEyeRemovalRGBThresholding();
    return 0;
}