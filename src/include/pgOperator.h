//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002, The pgAdmin Development Team
// This software is released under the pgAdmin Public Licence
//
// pgOperator.h PostgreSQL Operator
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGOperator_H
#define PGOperator_H

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "pgObject.h"
#include "pgServer.h"
#include "pgDatabase.h"


class pgOperator : public pgSchemaObject
{
public:
    pgOperator(pgSchema *newSchema, const wxString& newName = wxString(""));
    ~pgOperator();

    void ShowTreeDetail(wxTreeCtrl *browser, frmMain *form=0, wxListCtrl *properties=0, wxListCtrl *statistics=0, ctlSQLBox *sqlPane=0);
    static void ShowTreeCollection(pgCollection *collection, frmMain *form, wxTreeCtrl *browser, wxListCtrl *properties, wxListCtrl *statistics, ctlSQLBox *sqlPane);
    wxString GetFullName() const;
    wxString GetLeftType() const { return leftType; }
    void iSetLeftType(const wxString& s) { leftType=s; }
    wxString GetRightType() const { return rightType; }
    void iSetRightType(const wxString& s) { rightType=s; }
    wxString GetResultType() { return resultType; }
    void iSetResultType(const wxString& s) { resultType=s; }
    wxString GetOperatorFunction() const { return operatorFunction; }
    void iSetOperatorFunction(const wxString& s) { operatorFunction=s; }
    wxString GetJoinFunction() const { return joinFunction; }
    void iSetJoinFunction(const wxString& s) { joinFunction=s; }
    wxString GetRestrictFunction() const { return restrictFunction; }
    void iSetRestrictFunction(const wxString& s) { restrictFunction=s; }
    wxString GetCommutator() const { return commutator; }
    void iSetCommutator(const wxString& s) { commutator=s; }
    wxString GetNegator() const { return negator; }
    void iSetNegator(const wxString& s) { negator=s; }
    wxString GetKind() const { return kind; }
    void iSetKind(const wxString& s) { kind=s; }
    wxString GetLeftSortOperator() const { return leftSortOperator; }
    void iSetLeftSortOperator(const wxString& s) { leftSortOperator=s; }
    wxString GetRightSortOperator() const { return  rightSortOperator; }
    void iSetRightSortOperator(const wxString& s) {  rightSortOperator=s; }
    wxString GetLessOperator() const { return lessOperator; }
    void iSetLessOperator(const wxString& s) { lessOperator=s; }
    wxString GetGreaterOperator() const { return  greaterOperator; }
    void iSetGreaterOperator(const wxString& s) {  greaterOperator=s; }
    bool GetHashJoins() const { return hashJoins; }
    void iSetHashJoins(bool b) {  hashJoins=b; }

    wxString GetSql(wxTreeCtrl *browser);

private:
    wxString leftType, rightType, resultType,
             operatorFunction, joinFunction, restrictFunction,
             commutator, negator, kind, 
             leftSortOperator, rightSortOperator, lessOperator, greaterOperator;
    bool hashJoins;
};

#endif