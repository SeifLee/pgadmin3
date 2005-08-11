//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id$
// Copyright (C) 2002 - 2005, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgAggregate.cpp - Aggregate class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "pgObject.h"
#include "pgAggregate.h"
#include "pgSchema.h"


pgAggregate::pgAggregate(pgSchema *newSchema, const wxString& newName)
: pgSchemaObject(newSchema, aggregateFactory, newName)
{
}

pgAggregate::~pgAggregate()
{
}

bool pgAggregate::DropObject(wxFrame *frame, wxTreeCtrl *browser, bool cascaded)
{
    wxString sql=wxT("DROP AGGREGATE ") + GetQuotedFullIdentifier() + wxT("(") + GetInputType() + wxT(")");
    if (cascaded)
        sql += wxT(" CASCADE");
    return GetDatabase()->ExecuteVoid(sql);
}

wxString pgAggregate::GetSql(wxTreeCtrl *browser)
{
    if (sql.IsNull())
    {
        sql = wxT("-- Aggregate: ") + GetQuotedFullIdentifier() + wxT("\n\n")
            + wxT("-- DROP AGGREGATE ") + GetQuotedFullIdentifier() + wxT("(") + GetInputType() + wxT(");")
            + wxT("\n\nCREATE AGGREGATE ") + GetQuotedFullIdentifier() 
            + wxT("(\n  BASETYPE=") + GetInputType()
            + wxT(",\n  SFUNC=") + GetStateFunction()
            + wxT(",\n  STYPE=") + GetStateType();
        AppendIfFilled(sql, wxT(",\n  FINALFUNC="), qtIdent(GetFinalFunction()));
        if (GetInitialCondition().length() > 0)
          sql += wxT(",\n  INITCOND=") + qtString(GetInitialCondition());
        AppendIfFilled(sql, wxT(",\n  SORTOP="), GetQuotedSortOp());

        sql += wxT("\n);\n")
            + GetOwnerSql(8, 0, wxT("AGGREGATE ") + GetQuotedFullIdentifier() 
                + wxT("(") + qtIdent(GetInputType())
                + wxT(")"));

        if (!GetComment().IsNull())
        {
            sql += wxT("COMMENT ON AGGREGATE ") + GetQuotedFullIdentifier() 
                + wxT("(") + qtIdent(GetInputType())
                + wxT(") IS ") + qtString(GetComment()) + wxT(";\n");
        }
    }

    return sql;
}


wxString pgAggregate::GetFullName() const
{
    return GetName() + wxT("(") + GetInputType() + wxT(")");
}



void pgAggregate::ShowTreeDetail(wxTreeCtrl *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
    if (properties)
    {
        CreateListColumns(properties);

        properties->AppendItem(_("Name"), GetName());
        properties->AppendItem(_("Input type"), GetInputType());
        properties->AppendItem(_("OID"), GetOid());
        properties->AppendItem(_("Owner"), GetOwner());
        properties->AppendItem(_("State type"), GetStateType());
        properties->AppendItem(_("State function"), GetStateFunction());
        properties->AppendItem(_("Final type"), GetFinalType());
        properties->AppendItem(_("Final function"), GetFinalFunction());
        if (GetConnection()->BackendMinimumVersion(8,1))
            properties->AppendItem(_("Sort operator"), GetSortOp());
        properties->AppendItem(_("Initial condition"), GetInitialCondition());
        properties->AppendItem(_("System aggregate?"), GetSystemObject());
        properties->AppendItem(_("Comment"), GetComment());
    }
}


pgObject *pgAggregate::Refresh(wxTreeCtrl *browser, const wxTreeItemId item)
{
    pgObject *aggregate=0;
    wxTreeItemId parentItem=browser->GetItemParent(item);
    if (parentItem)
    {
        pgObject *obj=(pgObject*)browser->GetItemData(parentItem);
        if (obj->IsCollection())
            aggregate = aggregateFactory.CreateObjects((pgCollection*)obj, 0, wxT("\n   AND aggfnoid::oid=") + GetOidStr());
    }
    return aggregate;
}


////////////////////////////////////////////////////////////////////////


pgObject *pgaAggregateFactory::CreateObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction)
{
    pgAggregate *aggregate=0;
    wxString sql=
        wxT("SELECT aggfnoid::oid, proname AS aggname, pg_get_userbyid(proowner) AS aggowner, aggtransfn,\n")
        wxT(        "aggfinalfn, proargtypes[0] AS aggbasetype, ")
        wxT(        "CASE WHEN (ti.typlen = -1 AND ti.typelem != 0) THEN (SELECT at.typname FROM pg_type at WHERE at.oid = ti.typelem) || '[]' ELSE ti.typname END as inputname, ")
        wxT(        "aggtranstype, ")
        wxT(        "CASE WHEN (tt.typlen = -1 AND tt.typelem != 0) THEN (SELECT at.typname FROM pg_type at WHERE at.oid = tt.typelem) || '[]' ELSE tt.typname END as transname, ")
        wxT(        "prorettype AS aggfinaltype, ")
        wxT(        "CASE WHEN (tf.typlen = -1 AND tf.typelem != 0) THEN (SELECT at.typname FROM pg_type at WHERE at.oid = tf.typelem) || '[]' ELSE tf.typname END as finalname, ")
        wxT(        "agginitval, description");

    if (collection->GetDatabase()->BackendMinimumVersion(8, 1))
    {
        sql += wxT(", oprname, opn.nspname as oprnsp\n")
               wxT("  FROM pg_aggregate ag\n")
               wxT("  LEFT OUTER JOIN pg_operator op ON op.oid=aggsortop\n")
               wxT("  LEFT OUTER JOIN pg_namespace opn ON opn.oid=op.oprnamespace");
    }
    else
        sql +=  wxT("\n  FROM pg_aggregate ag\n");


    pgSet *aggregates= collection->GetDatabase()->ExecuteSet(sql +
        wxT("  JOIN pg_proc pr ON pr.oid = ag.aggfnoid\n")
        wxT("  JOIN pg_type ti on ti.oid=proargtypes[0]\n")
        wxT("  JOIN pg_type tt on tt.oid=aggtranstype\n")
        wxT("  JOIN pg_type tf on tf.oid=prorettype\n")
        wxT("  LEFT OUTER JOIN pg_description des ON des.objoid=aggfnoid::oid\n")
        wxT(" WHERE pronamespace = ") + collection->GetSchema()->GetOidStr()
        + restriction
        + wxT("\n ORDER BY aggname"));

    if (aggregates)
    {
        while (!aggregates->Eof())
        {
            aggregate = new pgAggregate(collection->GetSchema(), aggregates->GetVal(wxT("aggname")));

            aggregate->iSetOid(aggregates->GetOid(wxT("aggfnoid")));
            aggregate->iSetOwner(aggregates->GetVal(wxT("aggowner")));
            if (aggregates->GetVal(wxT("inputname")) == wxT("any"))
                aggregate->iSetInputType(wxT("\"any\""));
            else
                aggregate->iSetInputType(aggregates->GetVal(wxT("inputname")));
            aggregate->iSetStateType(aggregates->GetVal(wxT("transname")));
            aggregate->iSetStateFunction(aggregates->GetVal(wxT("aggtransfn")));
            aggregate->iSetFinalType(aggregates->GetVal(wxT("finalname")));

            wxString final=aggregates->GetVal(wxT("aggfinalfn"));
            if (final != wxT("-"))
                aggregate->iSetFinalFunction(final);
            aggregate->iSetInitialCondition(aggregates->GetVal(wxT("agginitval")));
            aggregate->iSetComment(aggregates->GetVal(wxT("description")));
            if (collection->GetDatabase()->BackendMinimumVersion(8, 1))
            {
                wxString oprname=aggregates->GetVal(wxT("oprname"));
                if (!oprname.IsEmpty())
                {
                    wxString oprnsp=aggregates->GetVal(wxT("oprnsp"));
                    aggregate->iSetSortOp(collection->GetDatabase()->GetSchemaPrefix(oprnsp) + oprname);
                    aggregate->iSetQuotedSortOp(collection->GetDatabase()->GetQuotedSchemaPrefix(oprnsp)
                        + qtIdent(oprname));
                }
            }

            if (browser)
            {
                collection->AppendBrowserItem(browser, aggregate);
                            aggregates->MoveNext();
            }
            else
                break;
        }

        delete aggregates;
    }
    return aggregate;
}


#include "images/aggregate.xpm"
#include "images/aggregates.xpm"

pgaAggregateFactory::pgaAggregateFactory() 
: pgaFactory(__("Aggregate"), __("New Aggregate"), __("Create a new Aggregate."), aggregate_xpm)
{
}

pgaAggregateFactory aggregateFactory;
static pgaCollectionFactory cf(&aggregateFactory, __("Aggregates"), aggregates_xpm);
