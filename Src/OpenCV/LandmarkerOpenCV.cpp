#include "LandmarkerOpenCV.h"
#include "Log.h"
#include "Performance.h"

#include <QDir>

CLandmarkerOpenCV::CLandmarkerOpenCV(QObject *parent)
    : CLandmarker(parent),
      m_bInit(false)
{}

int CLandmarkerOpenCV::UpdateParameter(QString &szErr)
{
    int nRet = 0;
    m_bInit = false;

    QString szPath = m_pParameter->GetModelPath() + QDir::separator() + "Opencv";
    QDir d;
    if(!d.exists(szPath)) szPath = m_pParameter->GetModelPath();
    LOG_MODEL_ERROR("CLandmarkerOpenCV", "The model files path: %s",
                    szPath.toStdString().c_str());
    QString szFile = szPath + QDir::separator() + "lbfmodel.yaml";
    
    try{
        m_Facemark = cv::face::FacemarkLBF::create();
        m_Facemark->loadModel(szFile.toStdString());
    } catch (cv::Exception e) {
        szErr = "Load model fail:";
        szErr += e.msg.c_str();
        LOG_MODEL_ERROR("CLandmarkerOpenCV", szErr.toStdString().c_str());
        return -2;
    }
    
    m_bInit = true;
    return nRet;
}

int CLandmarkerOpenCV::Mark(const QImage &image, const QRect &face, QVector<QPointF> &points)
{
    int nRet = 0;
    if(!m_bInit) return -1;
    if(!m_Facemark) return -2;
    if(image.isNull()) return -1;

    QImage img = image;
    if(img.format() != QImage::Format_RGB888)
    {
        PERFORMANCE_START(OpenCVMark)
        img = img.convertToFormat(QImage::Format_RGB888);     
        PERFORMANCE_ADD_TIME(OpenCVMark,
                             "conver format, image width:"
                             + QString::number(image.width())
                             + ";Height:"
                             + QString::number(image.height()))
    }

    // Load image
	cv::Mat frame(img.height(), img.width(), CV_8UC3, img.bits());

    std::vector<cv::Rect> faces;
    cv::Rect rect(face.x(), face.y(), face.width(), face.height());
    faces.push_back(rect);

    std::vector< std::vector<cv::Point2f> > landmarks;
    bool success = m_Facemark->fit(frame, faces, landmarks);
    if(!success)
    {
        LOG_MODEL_ERROR("CLandmarkerOpenCV", "CLandmarkerOpenCV::Mark fail");
        return -3;
    }

    std::vector< std::vector<cv::Point2f> >::iterator it;
    foreach(auto lm, landmarks)
        foreach(auto p, lm)
        {
            QPointF point(p.x, p.y);
            points.push_back(point);
        }

    return nRet;
}
