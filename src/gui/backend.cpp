#include "backend.hpp"

QString neroshop::Backend::urlToLocalFile(const QUrl& url) const {
    return url.toLocalFile();
}

void neroshop::Backend::copyTextToClipboard(const QString& text) {
    QClipboard * clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
    std::cout << "Copied text to clipboard\n";
}
