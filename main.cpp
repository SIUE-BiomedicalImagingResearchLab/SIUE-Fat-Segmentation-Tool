#include "mainwindow.h"
#include "application.h"

#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    int ret = -1;

    try
    {
        Application a(argc, argv);
        QCoreApplication::setOrganizationName("Southern Illinois University Edwardsville");
        QCoreApplication::setApplicationName("Visceral Fat Validation");
        QCoreApplication::setApplicationVersion(QT_VERSION_STR);
        MainWindow w;

        /*int dims[] = { 7, 7, 7};
        int datatype = CV_16SC1;
        cv::Mat test(3, dims, datatype, cv::Scalar(0));

        QFile file("C:/Users/addis/Desktop/matrix.txt");
        if (!file.open(QIODevice::WriteOnly))
            return 0;

        QTextStream stream(&file);

        for (int z = 0; z < 7; ++z)
        {
            for (int x = 0; x < 7; ++x)
            {
                for (int y = 0; y < 7; ++y)
                {
                    stream << test.at<short>(x, y, z) << " ";
                }
                stream << "\n";
            }
            stream << "-------------------\n\n\n--------------------------------\n";
        }

        cv::Range region[] = { cv::Range::all(), cv::Range(2,4), cv::Range(6, 7) };
        cv::Mat roi(test, region);

        roi.setTo(360);

        stream << "Second Test\n";
        for (int z = 0; z < 7; ++z)
        {
            for (int x = 0; x < 7; ++x)
            {
                for (int y = 0; y < 7; ++y)
                {
                    stream << test.at<short>(x, y, z) << " ";
                }
                stream << "\n";
            }
            stream << "-------------------\n\n\n--------------------------------\n";
        }

        cv::Range region2[] = { cv::Range(3, 6), cv::Range(0, 3), cv::Range(1, 4) };
        cv::Mat roi2 = test(region2);

        //roi.setTo(360);
        //roi2.data

        short testing[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
        int dims2[] = {3, 3, 3};
        cv::Mat testing2(3, dims2, CV_16SC1, testing);

        testing2.copyTo(roi2);
        //roi2.data = reinterpret_cast<uchar *>(testing);

        stream << "Continuous: " << roi2.isContinuous() << " and Regular matrix: " << test.isContinuous() << "\n";

        stream << "Third Test\n";
        for (int z = 0; z < 7; ++z)
        {
            for (int y = 0; y < 7; ++y)
            {
                for (int x = 0; x < 7; ++x)
                {
                    stream << test.at<short>(z, y, x) << " ";
                }
                stream << "\n";
            }
            stream << "-------------------\n\n\n--------------------------------\n";
        }

        stream << test.dims << " " << test.size[0] << " " << test.size[1] << " " << test.size[2] << "\n";
        file.close();*/


        w.show();
        ret = a.exec();
    }
    catch (const std::logic_error &e)
    {
        QMessageBox::critical(NULL, "Logic Error", e.what());
        qCritical() << "Logic Error: " << e.what();
    }
    catch (const std::runtime_error &e)
    {
        QMessageBox::critical(NULL, "Runtime Error", e.what());
        qCritical() << "Runtime Error: " << e.what();
    }
    catch (const Exception &e)
    {
        QMessageBox::critical(NULL, e.title(), e.message());
        qCritical() << "Error " << e.title() << " : " << e.message();
    }
    catch (const cv::Exception &e)
    {
        QMessageBox::critical(NULL, "OpenCV Error", e.what());
        qCritical() << "OpenCV Error: " << e.what();
    }
    catch (...)
    {
        QMessageBox::critical(NULL, "Unknown Error", "An unknown error occurred.");
        qCritical() << "An unknown error was thrown";
    }

    return ret;
}
