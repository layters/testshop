#include "extractor.hpp"

#include "../util.hpp" // neroshop::filesystem::

#if defined(NEROSHOP_USE_QT)
#include <QDebug>
#include <QProcess>
#include <QString>
#else
#include <cstdlib> // std::system
#include <cstdio> // std::remove, FILE
#endif

#include <cassert> // assert

void neroshop::tools::extractor::extract_tar(const std::string& filename) {
    std::cout << "extracting " << "tor/ from " << filename << "\n";
    
    std::string folder = filename.substr(0, filename.find_last_of("\\/")); // get path from filename
    assert(neroshop::filesystem::is_directory(folder));

    #if defined(NEROSHOP_USE_QT)
    QString fileName = QString::fromStdString(filename);
    QString extractDir = QString::fromStdString(folder);
    QString wildcards = "tor/*";

    QProcess process;
    process.setProgram("tar");
        
    QStringList arguments;
    arguments << "-xzf" << fileName << "-C" << extractDir << "--wildcards" << wildcards;
    process.setArguments(arguments);

    process.start();
    process.waitForFinished();

    if(process.exitCode() != 0) {
        qDebug() << process.readAllStandardError();
    }    
    #elif defined(__gnu_linux__) && !defined(NEROSHOP_USE_QT)
    std::system(std::string("tar -xzf " + filename + " -C " + folder + " --wildcards tor/*").c_str());
    #endif
    // Check if tor/tor file was extracted or not
    std::string out_file { folder + "/" + "tor/tor" };
    
    if(!neroshop::filesystem::is_file(out_file)) {
        std::cout << "Error extracting tar\n"; return;// or try a different method?
    }
    std::cout << "Extraction completed!\n";
    std::remove(filename.c_str()); // Delete the tar.gz file after we're done extracting
}

void neroshop::tools::extractor::extract_zip(const std::string& filename) {
}
