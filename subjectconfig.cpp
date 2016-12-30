#include "subjectconfig.h"

SubjectConfig::SubjectConfig() : imageUpperInferior(0), imageUpperSuperior(0), imageLowerInferior(0), imageLowerSuperior(0),
                                EATInferior(0), EATSuperior(0), PATInferior(0), PATSuperior(0), PAATInferior(0), PAATSuperior(0),
                                VATInferior(0), VATSuperior(0), SCATInferior(0), SCATSuperior(0)
{

}

SubjectConfig::SubjectConfig(QString filename) : SubjectConfig()
{
    if (!load(filename))
        EXCEPTION("Error loading subject configuration", "There was an error while loading the subject configuration data contained in " + filename);
}

bool SubjectConfig::load(QString filename)
{
    // Do not load if filename is empty
    if (filename.isEmpty())
        return false;

    if (!util::fileExists(filename))
    {
        qDebug() << "Unable to load config file for NIFTI images. Config file " << filename << " does not exist.";
        return false;
    }

    QFile file(filename);
    QDomDocument doc;

    // Open the file and load the XML document
    if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
    {
        qDebug() << "There was an issue opening the config file " << filename;
        return false;
    }

    QDomNodeList list = doc.elementsByTagName("imageUpper");
    if (list.count() != 1)
    {
        qDebug() << "Multiple imageUpper tags in config file";
        file.close();
        return false;
    }

    QDomElement element = list.item(0).toElement();
    imageUpperInferior = element.attribute("inferiorSlice", "-1").toInt();
    imageUpperSuperior = element.attribute("superiorSlice", "-1").toInt();

    if (imageUpperInferior < 0 || imageUpperSuperior < 0)
    {
        qDebug() << "Invalid inferior or superior slice value for upper NIFTI image: " << imageUpperInferior << " " << imageUpperSuperior;
        file.close();
        return false;
    }

    list = doc.elementsByTagName("imageLower");
    if (list.count() != 1)
    {
        qDebug() << "Multiple imageLower tags in config file";
        file.close();
        return false;
    }

    element = list.item(0).toElement();
    imageLowerInferior = element.attribute("inferiorSlice", "-1").toInt();
    imageLowerSuperior = element.attribute("superiorSlice", "-1").toInt();

    if (imageLowerInferior < 0 || imageLowerSuperior < 0)
    {
        qDebug() << "Invalid inferior or superior slice value for lower NIFTI image: " << imageLowerInferior << " " << imageLowerSuperior;
        file.close();
        return false;
    }

    file.close();
    return true;
}
