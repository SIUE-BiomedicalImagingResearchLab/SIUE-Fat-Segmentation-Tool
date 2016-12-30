#ifndef SUBJECTCONFIG_H
#define SUBJECTCONFIG_H

#include <QFile>
#include <QString>
#include <Qtxml>

#include "exception.h"
#include "util.h"

class SubjectConfig
{
public:
    int imageUpperInferior, imageUpperSuperior;
    int imageLowerInferior, imageLowerSuperior;

    int EATInferior, EATSuperior;
    int PATInferior, PATSuperior;
    int PAATInferior, PAATSuperior;
    int VATInferior, VATSuperior;
    int SCATInferior, SCATSuperior;

    SubjectConfig();
    SubjectConfig(QString filename);

    bool load(QString filename);
};

#endif // SUBJECTCONFIG_H
