//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002, The pgAdmin Development Team
// This software is released under the pgAdmin Public Licence
//
// dlgLanguage.cpp - PostgreSQL Language Property
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "dlgLanguage.h"
#include "pgLanguage.h"

// Images
#include "images/language.xpm"


// pointer to controls
#define chkTrusted      CTRL("chkTrusted", wxCheckBox)
#define cbHandler       CTRL("cbHandler", wxComboBox)
#define cbValidator     CTRL("cbValidator", wxComboBox)


BEGIN_EVENT_TABLE(dlgLanguage, dlgSecurityProperty)
    EVT_TEXT(XRCID("txtName"),                      dlgLanguage::OnChange)
    EVT_COMBOBOX(XRCID("cbHandler"),                dlgLanguage::OnChange)
END_EVENT_TABLE();


dlgLanguage::dlgLanguage(frmMain *frame, pgLanguage *node)
: dlgSecurityProperty(frame, node, wxT("dlgLanguage"), wxT("USAGE"), "U")
{
    SetIcon(wxIcon(language_xpm));
    language=node;

    txtOID->Disable();
}


pgObject *dlgLanguage::GetObject()
{
    return language;
}


int dlgLanguage::Go(bool modal)
{
    AddGroups();
    AddUsers();
    if (language)
    {
        // edit mode
        txtName->SetValue(language->GetName());
        txtOID->SetValue(NumToStr((long)language->GetOid()));
        chkTrusted->SetValue(language->GetTrusted());
        cbHandler->Append(language->GetHandlerProc());
        cbHandler->SetSelection(0);
        wxString val=language->GetValidatorProc();
        if (!val.IsEmpty())
        {
            cbValidator->Append(val);
            cbValidator->SetSelection(0);
        }

        txtName->Disable();
        cbHandler->Disable();
        chkTrusted->Disable();
        cbValidator->Disable();
    }
    else
    {
        // create mode
        pgSet *set=connection->ExecuteSet(wxT(
            "SELECT nspname, proname, prorettype\n"
            "  FROM pg_proc p\n"
            "  JOIN pg_namespace nsp ON nsp.oid=pronamespace\n"
            " WHERE prorettype=2280 OR (prorettype=2278 and proargtypes[0]=26)"));
        if (set)
        {
            while (!set->Eof())
            {
                wxString nspname;
                if (nspname == wxT("public"))
                    nspname=wxT("");
                else
                    nspname += wxT(".");

                if (set->GetOid(wxT("prorettype")) == 2280)
                    cbHandler->Append(nspname + set->GetVal(wxT("proname")));
                else
                    cbValidator->Append(nspname + set->GetVal(wxT("proname")));
                set->MoveNext();
            }
            delete set;
        }
    }

    return dlgSecurityProperty::Go(modal);
}


pgObject *dlgLanguage::CreateObject(pgCollection *collection)
{
    wxString name=txtName->GetValue();

    pgObject *obj=pgLanguage::ReadObjects(collection, 0, wxT("\n   AND lanname ILIKE ") + qtString(name));
    return obj;
}


void dlgLanguage::OnChange(wxNotifyEvent &ev)
{
    if (language)
    {
    }
    else
    {
        wxString name=txtName->GetValue();

        EnableOK(!name.IsEmpty() && !cbHandler->GetValue().IsEmpty());
    }
}



wxString dlgLanguage::GetSql()
{
    wxString sql, name;
    name=txtName->GetValue();

    if (language)
    {
        // edit mode
    }
    else
    {
        // create mode
        sql = wxT("CREATE ");
        if (chkTrusted->GetValue())
            sql += wxT("TRUSTED ");
        sql += wxT("LANGUAGE ") + name + wxT("\n   HANDLER ") + qtIdent(cbHandler->GetValue());
        AppendIfFilled(sql, wxT("\n   VALIDATOR "), qtIdent(cbValidator->GetValue()));
        sql += wxT(";\n");

    }

    sql += GetGrant(wxT("-"), wxT("LANGUAGE ") + name);

    return sql;
}


