#ifndef TST_TELEMETRYMERGELOGIC_H
#define TST_TELEMETRYMERGELOGIC_H

#include <QObject>

class TestTelemetryMergeLogic : public QObject {
    Q_OBJECT

private slots:
    void compareBySort_ordersByTimestampAscending();
    void findInsertIndex_insertsIntoMiddle();
    void shouldAcceptAtViewportEdge_middleAcceptsOnlyWithinWindow();
    void shouldAcceptAtViewportEdge_bottomAcceptsNewerRows();
    void mergeFilteredInsertions_skipsWhenScrolling();
    void mergeFilteredInsertions_insertsMatchingRowsAtBottomEdge();
    void mergeFilteredInsertions_insertsIntoMiddleWindow();
    void mergeFilteredInsertions_skipsDuplicates();
};

#endif // TST_TELEMETRYMERGELOGIC_H
