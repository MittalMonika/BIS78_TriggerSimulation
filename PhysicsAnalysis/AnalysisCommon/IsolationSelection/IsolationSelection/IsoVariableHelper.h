/*
 Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOVARIABLEHELPER_H
#define ISOLATIONSELECTION_ISOVARIABLEHELPER_H


#include <xAODBase/IParticle.h>
#include <IsolationSelection/Defs.h>
#include <PATInterfaces/CorrectionCode.h>

#include <string>
#include <memory>
namespace CP {        
    class IsoVariableHelper;

    typedef std::unique_ptr<IsoVariableHelper> IsoHelperPtr;

    class IsoVariableHelper {
        public:
            CorrectionCode GetOrignalIsolation(const xAOD::IParticle*  particle, float& value) const;
            CorrectionCode GetIsolation(const xAOD::IParticle*  particle, float& value) const;
            CorrectionCode BackupIsolation(const xAOD::IParticle*  particle) const;
            CorrectionCode SetIsolation(xAOD::IParticle* P, float value) const;

            IsoType isotype() const;
            std::string name() const;

            IsoVariableHelper(IsoType type, const std::string& BackupPreFix);

        private:

            IsoType m_isoType;
            bool m_BackupIso;
            CharDecorator m_dec_IsoIsBackup;
            CharAccessor m_acc_IsoIsBackup;

            FloatAccessor m_acc_iso_variable;
            FloatDecorator m_dec_iso_variable;

            FloatAccessor m_acc_iso_backup;
            FloatDecorator m_dec_iso_backup;
    };
}

#endif
