/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// contact: jmaurer@cern.ch

#include "TrigGlobalEfficiencyCorrection/Calculator.h"
#include "TrigGlobalEfficiencyCorrection/ImportData.h"
#include "TrigGlobalEfficiencyCorrection/Lepton.h"
#include "TrigGlobalEfficiencyCorrection/Trigger.h"
#include "AsgTools/AsgMessaging.h"

#include <random>

using namespace TrigGlobEffCorr;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

Calculator::Helper::Helper(const std::vector<TrigDef>& defs) :
	m_n1L(std::count_if(defs.cbegin(), defs.cend(), [](const TrigDef& td)->bool{ return td.type&TT_SINGLELEPTON_FLAG;})),
	m_n2L(std::count_if(defs.cbegin(), defs.cend(), [](const TrigDef& td)->bool{ return td.type&TT_DILEPTON_FLAG;})),
	m_n3L(std::count_if(defs.cbegin(), defs.cend(), [](const TrigDef& td)->bool{ return td.type&TT_TRILEPTON_FLAG;})),
	m_nPhotonTriggers(std::count_if(defs.cbegin(), defs.cend(), [](const TrigDef& td)->bool{ return td.type&TT_PHOTON_FLAG;})),
	m_func(nullptr), m_defs(defs)
{
}

namespace TrigGlobEffCorr
{

template<typename Trig1L>
bool Calculator::Helper::bind_1x1L()
{
	auto n1 = count<Trig1L>();
	if(!n1 || n1!=m_defs.size()) return false;
	if(n1>1)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const flat_set<Trig1L>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Several1L, ::_1, ::_2, ::_3, get_all<Trig1L>(), ::_4);
	}
	else
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig1L, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One1L, ::_1, ::_2, ::_3, get<Trig1L>(0), ::_4);
	}
	return true;
}

template<>
bool Calculator::Helper::bind_1x1L<void>()
{
	auto nE = count<Trig1E>(), nM = count<Trig1MU>(), n1 = nE+nM;
	if(!n1 || n1!=m_defs.size()) return false;
	if(nE>1 || nM>1)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const flat_set<Trig1E>&, const flat_set<Trig1MU>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Several1L, ::_1, ::_2, ::_3, get_all<Trig1E>(), get_all<Trig1MU>(), ::_4);
	}
	else if(nE>0 && nM>0)
	{
		m_func = std::bind(&Calculator::globalEfficiency_Two1L, ::_1, ::_2, ::_3, get<Trig1E>(0), get<Trig1MU>(0), ::_4);
	}
	else if(!nE) return bind_1x1L<Trig1MU>();
	else return bind_1x1L<Trig1E>();
	return true;
}

template<typename Trig2L>
bool Calculator::Helper::bind_1x2L_singleFlavour()
{
	using Trig1L = typename Trig2L::companionTrig1LType;
	auto n2 = count<Trig2L>(), n1 = count<Trig1L>();
	if(n2!=1 || n2+n1!=m_defs.size()) return false;
	if(n1 > 0)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig2L, const flat_set<Trig1L>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One2LSeveral1L, ::_1, ::_2, ::_3, get<Trig2L>(0), get_all<Trig1L>(), ::_4);
	}
	else
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig2L, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One2L, ::_1, ::_2, ::_3, get<Trig2L>(0), ::_4);
	}
	return true;
}

template<typename Trig2L>
bool Calculator::Helper::bind_1x2L()
{
	using Trig1L = typename Trig2L::companionTrig1LType;
	std::size_t n2 = count<Trig2L>(), n1 = count<Trig1E>()+count<Trig1MU>(), n1sf = count<Trig1L>();
	if(n2!=1 || n2+n1!=m_defs.size()) return false;
	if(n1 > n1sf)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig2L, const flat_set<Trig1E>&, const flat_set<Trig1MU>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One2LSeveral1L, ::_1, ::_2, ::_3, get<Trig2L>(0), get_all<Trig1E>(), get_all<Trig1MU>(), ::_4);
		return true;
	}
	return bind_1x2L_singleFlavour<Trig2L>();
}

template<>
bool Calculator::Helper::bind_1x2L<TrigEMU>()
{
	auto n2 = count<TrigEMU>(), n1 = count<Trig1E>()+count<Trig1MU>();
	if(n2!=1 || n2+n1!=m_defs.size()) return false;
	if(n1 > 0)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const TrigEMU, const flat_set<Trig1E>&, const flat_set<Trig1MU>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One2LSeveral1L, ::_1, ::_2, ::_3, get<TrigEMU>(0), get_all<Trig1E>(), get_all<Trig1MU>(), ::_4);
	}
	else
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const TrigEMU, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One2L, ::_1, ::_2, ::_3, get<TrigEMU>(0), ::_4);
	}
	return true;
}

template<typename Trig2L1, typename Trig2L2>
bool Calculator::Helper::bind_2x2L_singleFlavour()
{
	static_assert((Trig2L1::object()==Trig2L2::object()) && !Trig2L1::mixed() && !Trig2L2::mixed(), "");
	using Trig1L = typename Trig2L1::companionTrig1LType;
	constexpr bool sameType = (Trig2L1::type()==Trig2L2::type());
	auto n2 = count<Trig2L1>()+(sameType?0:count<Trig2L2>()), n1 = count<Trig1L>();
	if(n2!=2 || n2+n1!=m_defs.size()) return false;
	using Trig2Lsym = typename std::conditional<Trig2L1::is2Lasym(), Trig2L2, Trig2L1>::type;
	using Trig2L = typename std::conditional<Trig2L1::is2Lasym(), Trig2L1, Trig2L2>::type;
	using fnptr = bool(Calculator::*)(const LeptonList& leptons, unsigned runNumber, const Trig2L, const Trig2Lsym, const flat_set<Trig1L>&, Efficiencies&);
	m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Two2LSeveral1L, ::_1, ::_2, ::_3, get<Trig2L>(0), get<Trig2Lsym>(sameType*1), get_all<Trig1L>(), ::_4);
	return true;
}

template<typename Trig2E, typename Trig2MU>
bool Calculator::Helper::bind_3x2L()
{
	static_assert(Trig2E::object()==xAOD::Type::Electron && Trig2MU::object()==xAOD::Type::Muon, "");
	std::size_t n2E = count<Trig2E>(), n2M = count<Trig2MU>(), nEM = count<TrigEMU>(), n1 = count<Trig1E>()+count<Trig1MU>();
	if(n2E>1 || n2M>1 || nEM>1 || n2E+n2M+nEM+n1!=m_defs.size()) return false;
	if(n1 > 0)
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig2E, const Trig2MU, 
			const TrigEMU, const flat_set<Trig1E>&, const flat_set<Trig1MU>&, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Three2LSeveral1L, ::_1, ::_2, ::_3, get<Trig2E>(0), get<Trig2MU>(0), 
			get<TrigEMU>(0), get_all<Trig1E>(), get_all<Trig1MU>(), ::_4);
	}
	else
	{
		using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig2E, const Trig2MU, const TrigEMU, Efficiencies&);
		m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Three2L, ::_1, ::_2, ::_3, get<Trig2E>(0), get<Trig2MU>(0), get<TrigEMU>(0), ::_4);
	}
	return true;
}

template<typename Trig2E, typename Trig2MU>
bool Calculator::Helper::bind_6x2L()
{
	static_assert(Trig2E::object()==xAOD::Type::Electron && Trig2MU::object()==xAOD::Type::Muon, "");
	constexpr bool two2Esym = std::is_same<Trig2E, Trig2Esym>::value, two2MUsym = std::is_same<Trig2MU, Trig2MUsym>::value;
	auto n2E = count<Trig2Esym>() + (two2Esym? 0 : count<Trig2E>());
	auto n2M = count<Trig2MUsym>() + (two2MUsym? 0 : count<Trig2MU>());
	auto nEM = count<TrigEMU>(), n1 = count<Trig1E>()+count<Trig1MU>();
	if(n2E>2 || n2M>2 || count<Trig2Easym>()>1 || count<Trig2MUasym>()>1 || nEM>2 || n2E+n2M+nEM+n1!=m_defs.size()) return false;
	Trig2Esym trig2Esym = get<Trig2Esym>(two2Esym? 1 : 0);
	Trig2MUsym trig2MUsym = get<Trig2MUsym>(two2MUsym? 1 : 0);
	using fnptr =  bool(Calculator::*)(const LeptonList&, unsigned, const Trig2E, const Trig2Esym, const Trig2MU, const Trig2MUsym, 
		const TrigEMU, const TrigEMU, const flat_set<Trig1E>&, const flat_set<Trig1MU>&, Efficiencies&);
	m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Six2LSeveral1L, ::_1, ::_2, ::_3, get<Trig2E>(0), trig2Esym, get<Trig2MU>(0), trig2MUsym, 
		get<TrigEMU>(0), get<TrigEMU>(1), get_all<Trig1E>(), get_all<Trig1MU>(), ::_4);
	return true;
}

template<typename Trig3L>
bool Calculator::Helper::bind_1x3L()
{
	std::size_t n3 = count<Trig3L>();
	if(n3!=1 || n3!=m_defs.size()) return false;
	using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig3L, Efficiencies&);
	m_func = std::bind<fnptr>(&Calculator::globalEfficiency_One3L, ::_1, ::_2, ::_3, get<Trig3L>(0), ::_4);
	return true;
}
   
template<typename Trig3L1, typename Trig3L2>
bool Calculator::Helper::bind_2x3L()
{
	static_assert((Trig3L1::type()!=Trig3L2::type()) && Trig3L1::is3Lmix() && Trig3L2::is3Lmix(), "");
	std::size_t n31 = count<Trig3L1>(), n32 = count<Trig3L2>();
	if(n31!=1 || n32!=1 || n31+n32!=m_defs.size()) return false;
	using fnptr = bool(Calculator::*)(const LeptonList&, unsigned, const Trig3L1, const Trig3L2, Efficiencies&);
	m_func = std::bind<fnptr>(&Calculator::globalEfficiency_Two3L, ::_1, ::_2, ::_3, get<Trig3L1>(0), get<Trig3L2>(0), ::_4);
	return true;
}

template<typename Trig>
Trig Calculator::Helper::get(unsigned index) const
{
	unsigned discarded = 0;
	for(auto& def : m_defs)
	{
		if(def.type != Trig::type()) continue;
		if(index < ++discarded)	
		{
			Trig t;
			t.set_definition(def);
			return t;
		}
	}
	return Trig();
}

template<typename Trig>
auto Calculator::Helper::get_all() const -> flat_set<Trig>
{
	flat_set<Trig> trigs;
	for(auto& def : m_defs)
	{
		if(def.type != Trig::type()) continue;
		Trig t;
		t.set_definition(def);
		trigs.emplace(t);
	}
	return trigs;
}

bool Calculator::Helper::duplicates() const
{
	for(auto itr1=m_defs.cbegin(); itr1!=m_defs.cend(); ++itr1)
		for(auto itr2=itr1+1; itr2!=m_defs.cend(); ++itr2)
			if(itr1->type==itr2->type && itr1->leg==itr2->leg) return true;
	return false;
}

}

Calculator::Calculator(TrigGlobalEfficiencyCorrectionTool& parent, unsigned nPeriodsToReserve) :
	asg::AsgMessaging(&parent),
	m_parent(&parent)
{
	msg().setLevel(parent.msg().level());
	m_periods.reserve(nPeriodsToReserve);
}
		
bool Calculator::addPeriod(ImportData& data, const std::pair<unsigned,unsigned>& boundaries, const std::string& combination, 
	bool useToys, bool useDefaultElectronTools, std::size_t& uniqueElectronLeg, bool useDefaultPhotonTools, std::size_t& uniquePhotonLeg)
{
	bool success = true;
	m_parent = data.getParent();
	
	auto triggers = data.parseTriggerString(combination,success);
	if(!success) return false;

	uniqueElectronLeg = uniquePhotonLeg = 0;
	bool severalElectronLegs = false, severalPhotonLegs = false;
	for(auto& trig : triggers)
	{
		switch(trig.type)
		{
		case TT_SINGLE_MU: case TT_2MU_ASYM: case TT_2MU_SYM: case TT_3MU_ASYM: case TT_3MU_SYM:
			break;
		case TT_SINGLE_E: case TT_2E_SYM: case TT_EMU: case TT_3E_SYM: case TT_E_2MU_ASYM: case TT_E_2MU_SYM:
			if(!severalElectronLegs)
			{
				if(uniqueElectronLeg && uniqueElectronLeg!=trig.leg[0]) severalElectronLegs = true;
				uniqueElectronLeg = trig.leg[0];
			}
			break;
		case TT_SINGLE_PH: case TT_2PH_SYM: case TT_3PH_SYM:
			if(!severalPhotonLegs)
			{
				if(uniquePhotonLeg && uniquePhotonLeg!=trig.leg[0]) severalPhotonLegs = true;
				uniquePhotonLeg = trig.leg[0];
			}
			break;
		case TT_2PH_ASYM: case TT_3PH_HALFSYM: case TT_3PH_ASYM:
			severalPhotonLegs = true;
			break;
		default: // other defined triggers have >= 2 distinct electron legs
			severalElectronLegs = true;
		}
	}
	if(useDefaultElectronTools && severalElectronLegs)
	{
		ATH_MSG_ERROR("The property 'ListOfLegsPerTool' needs to be filled as the specified trigger combination involves several electron trigger legs");
		return false;
	}
	if(useDefaultPhotonTools && severalPhotonLegs)
	{
		ATH_MSG_ERROR("The property 'ListOfLegsPerTool' needs to be filled as the specified trigger combination involves several photon trigger legs");
		return false;
	}
	
	/// Choose the appropriate function to compute efficiencies for this particular trigger combination
	
	Helper helper(triggers);
	if(helper.duplicates())
	{
		ATH_MSG_ERROR("The following combination of triggers contains duplicates: " << combination);
		return false;
	}
	if(!useToys)
	{	
		const unsigned n1L = helper.m_n1L, n2L = helper.m_n2L, n3L = helper.m_n3L;
		if(!(n1L+n2L+n3L))
		{
			ATH_MSG_ERROR("The trigger combination is empty!");
			return false;
		}
		else if(n1L && !(n2L+n3L)) // only single-lepton triggers
		{
			success = helper.bind_1x1L();
		}
		else if(!helper.m_nPhotonTriggers)
		{
			
			if(n2L==1 && !n3L) // one dilepton trigger (+ single-lepton triggers)
			{
				success = helper.bind_1x2L<TrigEMU>() || helper.bind_1x2L<Trig2Esym>() || helper.bind_1x2L<Trig2Easym>()
					|| helper.bind_1x2L<Trig2MUsym>() || helper.bind_1x2L<Trig2MUasym>();
			}
			else if(n2L>=2 && n2L<=6 && !n3L) // several dilepton triggers (+ single-lepton triggers)
			{
				success = helper.bind_3x2L<Trig2Esym,Trig2MUsym>() || helper.bind_3x2L<Trig2Easym,Trig2MUsym>() 
					|| helper.bind_3x2L<Trig2Esym,Trig2MUasym>() || helper.bind_3x2L<Trig2Easym,Trig2MUasym>() 
					|| helper.bind_6x2L<Trig2Esym,Trig2MUsym>() || helper.bind_6x2L<Trig2Easym,Trig2MUsym>()
					|| helper.bind_6x2L<Trig2Esym,Trig2MUasym>() || helper.bind_6x2L<Trig2Easym,Trig2MUasym>();
			}
			else if(n3L==1 && !n2L && !n1L)
			{
				success = helper.bind_1x3L<Trig3Esym>() || helper.bind_1x3L<Trig3Ehalfsym>()
					|| helper.bind_1x3L<Trig3MUsym>() || helper.bind_1x3L<Trig3MUhalfsym>()
					|| helper.bind_1x3L<Trig2EMUsym>() || helper.bind_1x3L<Trig2EMUasym>()
					|| helper.bind_1x3L<TrigE2MUsym>() || helper.bind_1x3L<TrigE2MUasym>();			
			}
			else if(n3L==2 && !n2L && !n1L)
			{
				success = helper.bind_2x3L<Trig2EMUsym,TrigE2MUsym>() || helper.bind_2x3L<Trig2EMUsym,TrigE2MUasym>()
					|| helper.bind_2x3L<Trig2EMUasym,TrigE2MUsym>() || helper.bind_2x3L<Trig2EMUasym,TrigE2MUasym>();
			}
		}
		else if(helper.m_nPhotonTriggers == triggers.size())
		{
			if(n2L==1 && !n3L) // one diphoton trigger (+ single-photon triggers)
			{
				success = helper.bind_1x2L_singleFlavour<Trig2PHsym>() || helper.bind_1x2L_singleFlavour<Trig2PHasym>();
			}
			else if(n2L==2 && !n3L) // two diphoton triggers (+ single-photon triggers)
			{
				success = helper.bind_2x2L_singleFlavour<Trig2PHasym,Trig2PHsym>() || helper.bind_2x2L_singleFlavour<Trig2PHsym,Trig2PHsym>();
			}
			else if(n3L==1 && !n2L && !n1L)
			{
				success = helper.bind_1x3L<Trig3PHsym>() || helper.bind_1x3L<Trig3PHhalfsym>();	
			}
		}
		if(!helper.m_func)
		{
			ATH_MSG_ERROR("This trigger combination is currently not supported with an explicit formula (you may use toys instead, slower): " << combination);
			return false;
		}
	}
	else
	{
		if(!helper.m_nPhotonTriggers || helper.m_nPhotonTriggers==triggers.size())
		{
			helper.m_func = std::bind(&Calculator::globalEfficiency_Toys, ::_1, ::_2, ::_3, triggers, ::_4);
		}
		else
		{
			ATH_MSG_ERROR("Currently it is not possible to combine electron/muon and photon triggers: " << combination);
			return false;
		}
	}
	if(success)
	{
		if(m_parent->m_validTrigMatchTool)
		{
			if(!data.adaptTriggerListForTriggerMatching(triggers)) return false;
			m_periods.emplace_back(boundaries, std::move(helper.m_func), std::move(triggers));
		}
		else m_periods.emplace_back(boundaries, std::move(helper.m_func));
	}
	else
	{
		ATH_MSG_ERROR("Unspecified error occurred while trying to find the formula for the trigger combination " << combination);
	}
	return success;
}

const Calculator::Period* Calculator::getPeriod(unsigned runNumber) const
{
	auto period = std::find_if(m_periods.cbegin(), m_periods.cend(),
		[=](const Period& p) { return runNumber>=p.m_boundaries.first && runNumber<=p.m_boundaries.second; });
	if(period == m_periods.end())
	{
		ATH_MSG_ERROR("No trigger combination has been specified for run " << runNumber);
		return nullptr;
	}
	return &*period;
}

bool Calculator::compute(TrigGlobalEfficiencyCorrectionTool& parent, const LeptonList& leptons, unsigned runNumber, Efficiencies& efficiencies)
{
	m_parent = &parent;
	auto period = getPeriod(runNumber);
	if(!period) return false;
	m_cachedEfficiencies.clear();
	return period->m_formula && period->m_formula(this, leptons, runNumber, efficiencies);
}

bool Calculator::checkTriggerMatching(TrigGlobalEfficiencyCorrectionTool& parent, bool& matched, const LeptonList& leptons, unsigned runNumber)
{
	matched = false;
	m_parent = &parent;
	auto period = getPeriod(runNumber);
	if(!period) return false;
	if(!period->m_triggers.size())
	{
		ATH_MSG_ERROR("Empty list of triggers for run number " << runNumber);
		return false;
	}
	auto& trigMatchTool = m_parent->m_trigMatchTool;
	
	/// First, for each lepton, list the trigger leg(s) it is allowed to fire (depends on pT and selection tags)
	const unsigned nLep = leptons.size();
	std::vector<flat_set<std::size_t> > validLegs(leptons.size());
	for(unsigned i=0;i<nLep;++i)
	{
		if(!fillListOfLegsFor(leptons[i], period->m_triggers, validLegs[i])) return false;
	}
	
	/// Then for each trigger, call trigger matching tool for all possible (valid) lepton combinations
	std::vector<flat_set<std::size_t> > firedLegs;
	std::vector<const xAOD::IParticle*> trigLeptons;
	const std::size_t magicWordHLT = 0xf7b8b87ef2917d66;

	for(auto& trig : period->m_triggers)
	{
		/// Get trigger chain name with a "HLT_" prefix
		auto itr = m_parent->m_dictionary.find(trig.name ^ magicWordHLT);
		if(itr == m_parent->m_dictionary.end())
		{
			itr = m_parent->m_dictionary.emplace(trig.name ^ magicWordHLT, "HLT_" + m_parent->m_dictionary.at(trig.name)).first;
		}
		const std::string& chain = itr->second;
		
		unsigned nLegs = 0;
		if(trig.type & TT_SINGLELEPTON_FLAG) nLegs = 1;
		else if(trig.type & TT_DILEPTON_FLAG) nLegs = 2;
		else if(trig.type & TT_TRILEPTON_FLAG) nLegs = 3;
		else
		{
			ATH_MSG_ERROR("Unsupported trigger (type = " << std::hex << trig.type << std::dec << ") " << chain );
			return false;
		}
		firedLegs.resize(nLegs);
		trigLeptons.resize(nLegs);
		for(unsigned i0=0;i0<nLep;++i0)
		{
			firedLegs[0].swap(validLegs[i0]); /// borrow the set of legs that can be fired by that lepton
			trigLeptons[0] = leptons[i0].particle();
			if(nLegs == 1)
			{
				if(canTriggerBeFired(trig, firedLegs) /// enough lepton(s) on trigger plateau?
					&& trigMatchTool->match(trigLeptons, chain)) return (matched = true);
			}
			else for(unsigned i1=i0+1;i1<nLep;++i1)
			{
				firedLegs[1].swap(validLegs[i1]);
				trigLeptons[1] = leptons[i1].particle();
				if(nLegs == 2)
				{
					if(canTriggerBeFired(trig, firedLegs)
						&& trigMatchTool->match(trigLeptons, chain)) return (matched = true);
				}
				else for(unsigned i2=i1+1;i2<nLep;++i2)
				{
					firedLegs[2].swap(validLegs[i2]);
					trigLeptons[2] = leptons[i2].particle();
					if(canTriggerBeFired(trig, firedLegs)
						&& trigMatchTool->match(trigLeptons, chain)) return (matched = true);
					firedLegs[2].swap(validLegs[i2]);
				}
				firedLegs[1].swap(validLegs[i1]);
			}
			firedLegs[0].swap(validLegs[i0]); /// return the set of legs back to the main container
		}
	}
	return true;
}

Efficiencies Calculator::getCachedTriggerLegEfficiencies(const Lepton& lepton, unsigned runNumber, std::size_t leg, bool& success)
{
	auto insertion = m_cachedEfficiencies.emplace(std::make_pair(&lepton,leg),Efficiencies());
	Efficiencies& efficiencies = insertion.first->second;
	if(insertion.second)
	{
		bool cpSuccess = false;
		switch(lepton.type())
		{
		case xAOD::Type::Electron:
			cpSuccess = m_parent->getTriggerLegEfficiencies(lepton.electron(), runNumber, leg, lepton.tag(), efficiencies);
			break;
		case xAOD::Type::Muon:
			cpSuccess = m_parent->getTriggerLegEfficiencies(lepton.muon(), leg, lepton.tag(), efficiencies);
			break;
		case xAOD::Type::Photon:
			cpSuccess = m_parent->getTriggerLegEfficiencies(lepton.photon(), runNumber, leg, lepton.tag(), efficiencies);
			break;
		default: ATH_MSG_ERROR("Unsupported particle type");
		}
		if(!cpSuccess)
		{
			efficiencies.data() = -777.;
			efficiencies.mc() = -777.;
			success = false;
		}
	}
	if(efficiencies.mc()==-777.) success = false;
	return efficiencies;
}


template<typename Trig1L>
auto Calculator::globalEfficiency_One1L(const LeptonList& leptons, unsigned runNumber, const Trig1L trig, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One1L() at line " << __LINE__);
	if(!trig)
	{
		globalEfficiencies = {0.};
		return true;
	}
	globalEfficiencies = {1.};
	bool success = true;
	Efficiencies efficiencies;
	for(auto& lepton : leptons)
	{
		if(trig.irrelevantFor(lepton) || !aboveThreshold(lepton, trig())) continue;
		auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(), success);
		globalEfficiencies *= ~efficiencies;
	}
	globalEfficiencies = ~globalEfficiencies;
	return success;
}

bool Calculator::globalEfficiency_Two1L(const LeptonList& leptons, unsigned runNumber, const Trig1E trig1E, const Trig1MU trig1MU, Efficiencies& globalEfficiencies)
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Two1L() at line " << __LINE__);
	if(!trig1E) return globalEfficiency_One1L(leptons, runNumber, trig1MU, globalEfficiencies);
	if(!trig1MU) return globalEfficiency_One1L(leptons, runNumber, trig1E, globalEfficiencies);
	globalEfficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		std::size_t leg = Trig1E::relevantFor(lepton)? trig1E() : trig1MU();
		if(!aboveThreshold(lepton, leg)) continue;
		auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, leg, success);
		globalEfficiencies *= ~efficiencies;
	}
	globalEfficiencies = ~globalEfficiencies;
	return success;
}

template<typename Trig1L>
auto Calculator::globalEfficiency_Several1L(const LeptonList& leptons, unsigned runNumber, const flat_set<Trig1L>& trigs, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Several1L() at line " << __LINE__);
	if(trigs.size() == 1) return globalEfficiency_One1L(leptons, runNumber, *trigs.cbegin(), globalEfficiencies);
	if(!trigs.size())
	{
		globalEfficiencies = {0.};
		return true;
	}
	globalEfficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(Trig1L::irrelevantFor(lepton)) continue;
		std::size_t loosestLeg = getLoosestLegAboveThreshold(lepton, trigs, success);
		if(loosestLeg && success)
		{
			auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, loosestLeg, success);
			globalEfficiencies *= ~efficiencies;
		}
	}
	globalEfficiencies = ~globalEfficiencies;
	return success;
}

bool Calculator::globalEfficiency_Several1L(const LeptonList& leptons, unsigned runNumber, 
	const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Several1L() at line " << __LINE__);
	if(trigs1E.size()==1 && trigs1MU.size()==1)
	{
		return globalEfficiency_Two1L(leptons, runNumber, *trigs1E.cbegin(), *trigs1MU.cbegin(), globalEfficiencies);
	}
	if(!trigs1E.size()) return globalEfficiency_Several1L(leptons, runNumber, trigs1MU, globalEfficiencies);
	if(!trigs1MU.size()) return globalEfficiency_Several1L(leptons, runNumber, trigs1E, globalEfficiencies);
	globalEfficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		std::size_t loosestLeg = (Trig1E::relevantFor(lepton))?
			getLoosestLegAboveThreshold(lepton, trigs1E, success): 
			getLoosestLegAboveThreshold(lepton, trigs1MU, success);
		if(loosestLeg && success)
		{
			auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, loosestLeg, success);
			globalEfficiencies *= ~efficiencies;
		}
	}
	globalEfficiencies = ~globalEfficiencies;
	return success;
}

bool Calculator::globalEfficiency_One2L(const LeptonList& leptons, unsigned runNumber, const TrigEMU trig, Efficiencies& globalEfficiencies)
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2L() at line " << __LINE__);
	Efficiencies electronEfficiencies, muonEfficiencies;
	bool success = globalEfficiency_One1L(leptons, runNumber, trig.side<xAOD::Type::Electron>(), electronEfficiencies)
		&& globalEfficiency_One1L(leptons, runNumber, trig.side<xAOD::Type::Muon>(), muonEfficiencies);
	if(success) globalEfficiencies = electronEfficiencies * muonEfficiencies;
	else globalEfficiencies = {0.};
	return success;
}

template<typename Trig2Lsym>
auto Calculator::globalEfficiency_One2L(const LeptonList& leptons, unsigned runNumber, const Trig2Lsym trig , Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lsym::is2Lsym(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2L() at line " << __LINE__);
	globalEfficiencies = {0.};
	if(!trig) return true;
	Efficiencies singleInefficiencies(1.);
	bool success = true;
	int nt = 0;
	for(auto& lepton : leptons)
	{
		if(trig.irrelevantFor(lepton) || !aboveThreshold(lepton, trig())) continue;
		++nt;
		auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(), success);
		globalEfficiencies = ~efficiencies*globalEfficiencies + efficiencies*~singleInefficiencies;
		singleInefficiencies *= ~efficiencies;
	}
	return success;
}

template<typename Trig2Lasym>
auto Calculator::globalEfficiency_One2L(const LeptonList& leptons, unsigned runNumber, const Trig2Lasym trig, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lasym::is2Lasym(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2L() at line " << __LINE__);
	if(trig.symmetric()) return globalEfficiency_One2L(leptons, runNumber, trig.to_symmetric(), globalEfficiencies);
	globalEfficiencies = {0.};
	if(!trig) return true;
	Efficiencies singleInefficiencies[2] = {{1.},{1.}}, twoSingleInefficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig.irrelevantFor(lepton)) continue;
		Efficiencies efficiencies[2] = {{0.}, {0.}};
		int loosest = 0;
		if(aboveThreshold(lepton, trig(0)))
		{
			efficiencies[0] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(0), success);
			if(aboveThreshold(lepton, trig(1)))
			{
				efficiencies[1] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(1), success);
				loosest = m_parent->getLoosestLeg(lepton, trig(0), trig(1), success)==trig(1);
			}
		}
		else if(aboveThreshold(lepton, trig(1)))
		{
			efficiencies[1] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(1), success);
			loosest = 1;
		}
		else continue;
		const int tightest = 1 - loosest;
		globalEfficiencies = ~efficiencies[loosest]*globalEfficiencies 
			+ (efficiencies[loosest]-efficiencies[tightest])*~singleInefficiencies[tightest]
			+ efficiencies[tightest]*~twoSingleInefficiencies;
		twoSingleInefficiencies *= ~efficiencies[loosest];
		for(int i=0;i<2;++i) singleInefficiencies[i] *= ~efficiencies[i];
	}
	return success;
}

bool Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber,
	const TrigEMU trigEMU, const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	if(!(trigs1E.size() + trigs1MU.size()))
		return globalEfficiency_One2L(leptons, runNumber, trigEMU, globalEfficiencies);
	if(trigEMU.hiddenBy(trigs1E) || trigEMU.hiddenBy(trigs1MU))
		return globalEfficiency_Several1L(leptons, runNumber, trigs1E, trigs1MU, globalEfficiencies);
	Efficiencies efficiencies[4];
	bool success = globalEfficiency_Several1L(leptons, runNumber, trigs1E, efficiencies[0])
		&& globalEfficiency_Several1L(leptons, runNumber, trigs1MU, efficiencies[1])
		&& globalEfficiency_Several1L(leptons, runNumber, trigEMU.addTo(trigs1E), efficiencies[2])
		&& globalEfficiency_Several1L(leptons, runNumber, trigEMU.addTo(trigs1MU), efficiencies[3]);
	if(success)
	{
		globalEfficiencies = Efficiencies(1.) - ~efficiencies[0]*~efficiencies[1]
			+ (efficiencies[2]-efficiencies[0])*(efficiencies[3]-efficiencies[1]);
	}
	else globalEfficiencies = {0.};
	return success;
}

template<typename Trig2L, typename Trig1L>
inline auto Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
			const Trig2L trig2L, const Trig1L trig1L, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2L::is2L(Trig1L::object()) && Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L, flat_set<Trig1L>{trig1L}, globalEfficiencies);
}

template<typename Trig2Lsym, typename Trig1L>
auto Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
			const Trig2Lsym trig2L, const flat_set<Trig1L>& trigs1L, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lsym::is2Lsym(Trig1L::object()) && Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	if(!trigs1L.size()) return globalEfficiency_One2L(leptons, runNumber, trig2L, globalEfficiencies);
	if(!trig2L || trig2L.hiddenBy(trigs1L)) return globalEfficiency_Several1L(leptons, runNumber, trigs1L, globalEfficiencies);
	globalEfficiencies = {0.};
	Efficiencies twoSingleInefficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig2L.irrelevantFor(lepton)) continue;
		Efficiencies efficiencies1L(0.), efficiencies2L(0.);
		const Efficiencies* loosestLegEfficiency;
		std::size_t loosest1lepLeg = getLoosestLegAboveThreshold(lepton, trigs1L, success);
		if(loosest1lepLeg)
		{
			efficiencies1L = getCachedTriggerLegEfficiencies(lepton, runNumber, loosest1lepLeg, success);
			if(aboveThreshold(lepton, trig2L()))
			{
				efficiencies2L = getCachedTriggerLegEfficiencies(lepton, runNumber, trig2L(), success);
				loosestLegEfficiency = (m_parent->getLoosestLeg(lepton, trig2L(), loosest1lepLeg , success)==trig2L())? &efficiencies2L : &efficiencies1L;
			}
			else loosestLegEfficiency = &efficiencies1L;
		}
		else if(aboveThreshold(lepton, trig2L()))
		{
			efficiencies2L = getCachedTriggerLegEfficiencies(lepton, runNumber, trig2L(), success);
			loosestLegEfficiency = &efficiencies2L;
		}
		else continue;
		globalEfficiencies = ~(*loosestLegEfficiency)*globalEfficiencies + efficiencies1L;
		if(loosestLegEfficiency==&efficiencies2L) globalEfficiencies += ~twoSingleInefficiencies*(efficiencies2L - efficiencies1L);			
		twoSingleInefficiencies *= ~(*loosestLegEfficiency);
	}
	return success;
}

template<typename Trig2Lasym, typename Trig1L>
auto Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
			const Trig2Lasym trig2L, const flat_set<Trig1L>& trigs1L, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lasym::is2Lasym(Trig1L::object()) && Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	if(trig2L.symmetric()) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L.to_symmetric(), trigs1L, globalEfficiencies);
	if(!trigs1L.size()) return globalEfficiency_One2L(leptons, runNumber, trig2L, globalEfficiencies);
	if(!trig2L || trig2L.hiddenBy(trigs1L)) return globalEfficiency_Several1L(leptons, runNumber, trigs1L, globalEfficiencies);
	globalEfficiencies = {0.};
	Efficiencies twoSingleInefficiencies[2] = {{1.}, {1.}}, threeSingleInefficiencies = {1.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig2L.irrelevantFor(lepton)) continue;
		flat_set<std::size_t> validLegs;
		for(std::size_t leg : trig2L.legs) if(aboveThreshold(lepton, leg)) validLegs.insert(leg);
		Efficiencies efficiencies1L = {0.};
		std::size_t loosest1lepLeg = getLoosestLegAboveThreshold(lepton, trigs1L, success);
		if(loosest1lepLeg)
		{
			efficiencies1L = getCachedTriggerLegEfficiencies(lepton, runNumber, loosest1lepLeg, success);
			validLegs.insert(loosest1lepLeg);
		}
		if(!validLegs.size()) continue;
		auto looseLegs = m_parent->getTwoLoosestLegs(lepton,validLegs,success);
		auto efficienciesLoose = (looseLegs.first==loosest1lepLeg)? efficiencies1L : getCachedTriggerLegEfficiencies(lepton, runNumber, looseLegs.first, success);
		Efficiencies efficienciesMedium = {0.};
		if(validLegs.size()>=2)
			efficienciesMedium = (looseLegs.second==loosest1lepLeg)? efficiencies1L : getCachedTriggerLegEfficiencies(lepton, runNumber, looseLegs.second, success);
		globalEfficiencies = ~efficienciesLoose*globalEfficiencies + efficiencies1L;
		if(loosest1lepLeg!=looseLegs.first)
		{
			globalEfficiencies += (efficienciesLoose-efficienciesMedium)*~twoSingleInefficiencies[looseLegs.first==trig2L.legs[0]];
			if(loosest1lepLeg!=looseLegs.second) globalEfficiencies += (efficienciesMedium-efficiencies1L)*~threeSingleInefficiencies;
		}
		threeSingleInefficiencies *= ~efficienciesLoose;
		twoSingleInefficiencies[0] *= (looseLegs.first!=trig2L.legs[1])?~efficienciesLoose:~efficienciesMedium; // S1 v S3
		twoSingleInefficiencies[1] *= (looseLegs.first!=trig2L.legs[0])?~efficienciesLoose:~efficienciesMedium; // S2 v S3
	}
	return success;
}

template<typename Trig2E>
auto Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
		const Trig2E trig2E, const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2E::is2L(xAOD::Type::Electron), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	Efficiencies effs[2] = {{0.}, {0.}};
	bool success = globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2E, trigs1E, effs[0])
		&& globalEfficiency_Several1L(leptons, runNumber, trigs1MU, effs[1]);
	globalEfficiencies = ~(~effs[0] * ~effs[1]);
	return success;
}

template<typename Trig2MU>
auto Calculator::globalEfficiency_One2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
		const Trig2MU trig2MU, const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2MU::is2L(xAOD::Type::Muon), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One2LSeveral1L() at line " << __LINE__);
	Efficiencies effs[2] = {{0.}, {0.}};
	bool success = globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2MU, trigs1MU, effs[0])
		&& globalEfficiency_Several1L(leptons, runNumber, trigs1E, effs[1]);
	globalEfficiencies = ~(~effs[0] * ~effs[1]);
	return success;
}

template<typename Trig2Lsym, typename Trig1L> 
auto Calculator::globalEfficiency_Two2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
			const Trig2Lsym trig2L1, const Trig2Lsym trig2L2, const flat_set<Trig1L>& trigs1L, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lsym::is2Lsym(Trig1L::object()) && Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Two2LSeveral1L() at line " << __LINE__);
	if(!trig2L1 || trig2L1==trig2L2 || trig2L1.hiddenBy(trigs1L)) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L2, trigs1L, globalEfficiencies);
	if(!trig2L2 || trig2L2.hiddenBy(trigs1L)) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L1, trigs1L, globalEfficiencies);
	globalEfficiencies = {0.};
	Efficiencies singleInefficiencies{1.};
	Efficiencies efficiencies2Lsym[2] = {{0.},{0.}};
	
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig2L1.irrelevantFor(lepton)) continue;
		flat_set<std::size_t> validLegs;
		std::map<std::size_t, Efficiencies> efficiencies{{0,0.}};
		std::size_t loosest1lepLeg = getLoosestLegAboveThreshold(lepton, trigs1L, success);
		for(std::size_t leg : {loosest1lepLeg, trig2L1(), trig2L2()})
		{
			if(leg && aboveThreshold(lepton, leg))
			{
				validLegs.insert(leg);
				efficiencies.emplace(leg, getCachedTriggerLegEfficiencies(lepton, runNumber, leg, success));
			}
			else efficiencies.emplace(leg, 0.);
		}
		if(!validLegs.size()) continue;
		auto looseLegs = m_parent->getTwoLoosestLegs(lepton, validLegs, success);
		std::size_t lambda13 = (looseLegs.first!=trig2L2())? looseLegs.first : looseLegs.second;
		std::size_t lambda23 = (looseLegs.first!=trig2L1())? looseLegs.first : looseLegs.second;
		globalEfficiencies = globalEfficiencies*~efficiencies[looseLegs.first] + efficiencies[loosest1lepLeg];
		if(looseLegs.first==trig2L1()) globalEfficiencies += efficiencies2Lsym[1]*(efficiencies[trig2L1()] - efficiencies[looseLegs.second]);
		if(looseLegs.first==trig2L2()) globalEfficiencies += efficiencies2Lsym[0]*(efficiencies[trig2L2()] - efficiencies[looseLegs.second]);
		if(looseLegs.first!=loosest1lepLeg) globalEfficiencies += ~singleInefficiencies*(efficiencies[looseLegs.second] - efficiencies[loosest1lepLeg]);
		efficiencies2Lsym[0] = ~efficiencies[looseLegs.first]*efficiencies2Lsym[0] + efficiencies[lambda23];
		efficiencies2Lsym[1] = ~efficiencies[looseLegs.first]*efficiencies2Lsym[1] + efficiencies[lambda13];
		if(looseLegs.first==trig2L1()) efficiencies2Lsym[0] += (efficiencies[trig2L1()]-efficiencies[lambda23])*~singleInefficiencies;
		if(looseLegs.first==trig2L2()) efficiencies2Lsym[1] += (efficiencies[trig2L2()]-efficiencies[lambda13])*~singleInefficiencies;
		singleInefficiencies *= ~efficiencies[looseLegs.first];
	}
	return success;
}

template<typename Trig2Lasym, typename Trig2Lsym, typename Trig1L>
auto Calculator::globalEfficiency_Two2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
			const Trig2Lasym trig2Lasym, const Trig2Lsym trig2Lsym, const flat_set<Trig1L>& trigs1L, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2Lasym::is2Lasym(Trig1L::object()) && Trig2Lsym::is2Lsym(Trig1L::object()) && Trig1L::is1L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Two2LSeveral1L() at line " << __LINE__);
	if(!trig2Lasym || trig2Lasym.hiddenBy(trigs1L)) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2Lsym, trigs1L, globalEfficiencies);
	if(!trig2Lsym || trig2Lsym.hiddenBy(trigs1L)) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2Lasym, trigs1L, globalEfficiencies);
	if(trig2Lasym(0)==trig2Lsym() || trig2Lasym(1)==trig2Lsym())
	{
		ATH_MSG_ERROR("implementation -- does this function work properly when the two 2L triggers have one leg in common? Must be checked");
		return false;
	}
	if(trig2Lasym.symmetric()) return globalEfficiency_Two2LSeveral1L(leptons, runNumber, trig2Lasym.to_symmetric(), trig2Lsym, trigs1L, globalEfficiencies);
	globalEfficiencies = {0.};
	Efficiencies singleInefficiencies[3] = {{1.}, {1.}, {1.}};
	Efficiencies efficiencies2Lasym {0.}, efficiencies2Lsym[3] = {{0.},{0.},{0.}};
	
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig2Lasym.irrelevantFor(lepton)) continue;
		flat_set<std::size_t> validLegs;
		std::map<std::size_t, Efficiencies> efficiencies{{0,0.}};
		std::size_t loosest1lepLeg = getLoosestLegAboveThreshold(lepton, trigs1L, success);
		for(std::size_t leg : {trig2Lasym(0), trig2Lasym(1), trig2Lsym(), loosest1lepLeg})
		{
			if(leg && aboveThreshold(lepton, leg))
			{
				validLegs.insert(leg);
				efficiencies.emplace(leg, getCachedTriggerLegEfficiencies(lepton, runNumber, leg, success));
			}
			else efficiencies.emplace(leg, 0.);
		}
		if(!validLegs.size()) continue;
		
		const auto sortedLegs = m_parent->getSortedLegs(lepton, validLegs, success);
		if(!success) return false;
		std::size_t loosestLeg = sortedLegs[0];
		std::size_t secondLoosestLeg = validLegs.size()>=2 ? sortedLegs[1] : 0;
		std::size_t secondTightestLeg = validLegs.size()>=3 ? sortedLegs[2] : 0;
		std::size_t tightestLeg = validLegs.size()>=4 ? sortedLegs[3]: 0;
		std::size_t lambda124 = (loosestLeg!=trig2Lasym(1))? loosestLeg : secondLoosestLeg;
		std::size_t lambda134 = (loosestLeg!=trig2Lasym(0))? loosestLeg : secondLoosestLeg;
		std::size_t lambda234 = (loosestLeg!=trig2Lsym())? loosestLeg : secondLoosestLeg;
		std::size_t lambda14 = (lambda124!=trig2Lasym(0))? lambda124 : (lambda134!=trig2Lasym(1))? lambda134 : secondTightestLeg;
		std::size_t lambda24 = (lambda124!=trig2Lsym())? lambda124 : (lambda234!=trig2Lasym(1))? lambda234 : secondTightestLeg;
		std::size_t lambda34 = (lambda134!=trig2Lsym())? lambda134 : (lambda234!=trig2Lasym(0))? lambda234 : secondTightestLeg;
		std::size_t tau13=0, tau12=0, tau23=0;
		if(loosestLeg==trig2Lsym() || loosestLeg==trig2Lasym(0)) tau12 = (loosestLeg==trig2Lsym())? trig2Lasym(0) : trig2Lsym();
		else if(secondLoosestLeg==trig2Lsym() || secondLoosestLeg==trig2Lasym(0)) tau12 = (secondLoosestLeg==trig2Lsym())? trig2Lasym(0) : trig2Lsym();
		else if(secondTightestLeg==trig2Lsym() || secondTightestLeg==trig2Lasym(0)) tau12 = (secondTightestLeg==trig2Lsym())? trig2Lasym(0) : trig2Lsym();
		else if(tightestLeg==trig2Lsym() || tightestLeg==trig2Lasym(0)) tau12 = (tightestLeg==trig2Lsym())? trig2Lasym(0) : trig2Lsym();
		if(loosestLeg==trig2Lsym() || loosestLeg==trig2Lasym(1)) tau13 = (loosestLeg==trig2Lsym())? trig2Lasym(1) : trig2Lsym();
		else if(secondLoosestLeg==trig2Lsym() || secondLoosestLeg==trig2Lasym(1)) tau13 = (secondLoosestLeg==trig2Lsym())? trig2Lasym(1) : trig2Lsym();
		else if(secondTightestLeg==trig2Lsym() || secondTightestLeg==trig2Lasym(1)) tau13 = (secondTightestLeg==trig2Lsym())? trig2Lasym(1) : trig2Lsym();
		else if(tightestLeg==trig2Lsym() || tightestLeg==trig2Lasym(1)) tau13 = (tightestLeg==trig2Lsym())? trig2Lasym(1) : trig2Lsym();
		if(loosestLeg==trig2Lasym(0) || loosestLeg==trig2Lasym(1)) tau23 = (loosestLeg==trig2Lasym(0))? trig2Lasym(1) : trig2Lasym(0);
		else if(secondLoosestLeg==trig2Lasym(0) || secondLoosestLeg==trig2Lasym(1)) tau23 = (secondLoosestLeg==trig2Lasym(0))? trig2Lasym(1) : trig2Lasym(0);
		else if(secondTightestLeg==trig2Lasym(0) || secondTightestLeg==trig2Lasym(1)) tau23 = (secondTightestLeg==trig2Lasym(0))? trig2Lasym(1) : trig2Lasym(0);
		else if(tightestLeg==trig2Lasym(0) || tightestLeg==trig2Lasym(1)) tau23 = (tightestLeg==trig2Lasym(0))? trig2Lasym(1) : trig2Lasym(0);

		// can't use tightestLeg==trig2Lsym because it might also be 0
		globalEfficiencies = globalEfficiencies*~efficiencies[loosestLeg] + efficiencies[loosest1lepLeg]
			+ (efficiencies[tau13] - efficiencies[secondTightestLeg])*~singleInefficiencies[0]
			+ (efficiencies[tau12] - efficiencies[secondTightestLeg])*~singleInefficiencies[1]
			+ (efficiencies[tau23] - efficiencies[secondTightestLeg])*efficiencies2Lsym[2];
		if(loosestLeg==trig2Lsym()) globalEfficiencies += (efficiencies[trig2Lsym()]-efficiencies[secondLoosestLeg])*efficiencies2Lasym;
		else if(loosestLeg==trig2Lasym(1)) globalEfficiencies += (efficiencies[trig2Lasym(1)]-efficiencies[secondLoosestLeg])*efficiencies2Lsym[0];
		else if(loosestLeg==trig2Lasym(0)) globalEfficiencies += (efficiencies[trig2Lasym(0)]-efficiencies[secondLoosestLeg])*efficiencies2Lsym[1];
		if(secondTightestLeg && tightestLeg==loosest1lepLeg) // this works because loosest1lepLeg is 0 if not on plateau...
			globalEfficiencies += (efficiencies[secondTightestLeg]-efficiencies[tightestLeg])*~singleInefficiencies[2];

		efficiencies2Lasym = ~efficiencies[loosestLeg]*efficiencies2Lasym + efficiencies[lambda14];
		if(loosestLeg==trig2Lasym(0) || loosestLeg==trig2Lasym(1))
		{
			// note: secondLoosestLeg is valid because the loosest leg is either trig2Lasym(0) or trig2Lasym(1)
			efficiencies2Lasym += (efficiencies[loosestLeg]-efficiencies[secondLoosestLeg])*~singleInefficiencies[loosestLeg==trig2Lasym(0)]
				+ (efficiencies[secondLoosestLeg]-efficiencies[lambda14])*~singleInefficiencies[2];
		}
		efficiencies2Lsym[0] = ~efficiencies[lambda124]*efficiencies2Lsym[0] + efficiencies[lambda24];
		efficiencies2Lsym[1] = ~efficiencies[lambda134]*efficiencies2Lsym[1] + efficiencies[lambda34];
		efficiencies2Lsym[2] = ~efficiencies[loosestLeg]*efficiencies2Lsym[2] + efficiencies[lambda234];
		if(lambda124==trig2Lsym()) efficiencies2Lsym[0] += (efficiencies[trig2Lsym()]-efficiencies[lambda24])*~singleInefficiencies[0];
		if(lambda134==trig2Lsym()) efficiencies2Lsym[1] += (efficiencies[trig2Lsym()]-efficiencies[lambda34])*~singleInefficiencies[1];
		if(loosestLeg==trig2Lsym()) efficiencies2Lsym[2] += (efficiencies[trig2Lsym()]-efficiencies[lambda234])*~singleInefficiencies[2];
		singleInefficiencies[0] *= ~efficiencies[lambda124];
		singleInefficiencies[1] *= ~efficiencies[lambda134];
		singleInefficiencies[2] *= ~efficiencies[loosestLeg];
	}
	return success;
}

template<typename Trig3Lsym>
auto Calculator::globalEfficiency_One3L(const LeptonList& leptons, unsigned runNumber, const Trig3Lsym trig, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig3Lsym::is3Lsym(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One3L() at line " << __LINE__);
	globalEfficiencies = {0.};
	Efficiencies singleInefficiencies{1.}, efficiencies2L{0.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig.irrelevantFor(lepton) || !aboveThreshold(lepton, trig())) continue;
		auto efficiencies = getCachedTriggerLegEfficiencies(lepton, runNumber, trig(), success);
		globalEfficiencies = ~efficiencies*globalEfficiencies + efficiencies*efficiencies2L;
		efficiencies2L = ~efficiencies*efficiencies2L + efficiencies*~singleInefficiencies;
		singleInefficiencies *= ~efficiencies;
	}
	return success;
}

template<typename Trig3Lhalfsym>
auto Calculator::globalEfficiency_One3L(const LeptonList& leptons, unsigned runNumber, const Trig3Lhalfsym trig, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig3Lhalfsym::is3Lhalfsym(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One3L() at line " << __LINE__);
	if(trig.symmetric()) return globalEfficiency_One3L(leptons, runNumber, trig.to_symmetric(), globalEfficiencies);
	globalEfficiencies = {0.};
	Efficiencies singleInefficiencies[2] = {{1.}, {1.}}, twoSingleInefficiencies{1.};
	Efficiencies efficiencies2Lsym{0.}, efficiencies2Lasym{0.}, efficiencies2L2L{0.};
	bool success = true;
	for(auto& lepton : leptons)
	{
		if(trig.irrelevantFor(lepton)) continue;
		Efficiencies efficiencies[2] = {{0.}, {0.}};
		const int asym = 0, sym = 1;
		int loosestLeg;
		if(aboveThreshold(lepton, trig.asymLeg()))
		{
			efficiencies[asym] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig.asymLeg(), success);
			if(aboveThreshold(lepton, trig.symLeg()))
			{
				efficiencies[sym] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig.symLeg(), success);
				loosestLeg = m_parent->getLoosestLeg(lepton, trig.asymLeg(), trig.symLeg(), success)==trig.asymLeg()? asym : sym;
			}
			else loosestLeg = asym;
		}
		else if(aboveThreshold(lepton, trig.symLeg()))
		{
			efficiencies[sym] = getCachedTriggerLegEfficiencies(lepton, runNumber, trig.symLeg(), success);
			loosestLeg = sym;
		}
		else continue;
		Efficiencies delta = efficiencies[asym] - efficiencies[sym];
		if(loosestLeg == asym)
		{
			globalEfficiencies = ~efficiencies[asym]*globalEfficiencies + efficiencies[sym]*efficiencies2L2L + delta*efficiencies2Lsym;
			efficiencies2L2L = ~efficiencies[asym]*efficiencies2L2L + efficiencies[sym]*~twoSingleInefficiencies + delta*~singleInefficiencies[sym];
			efficiencies2Lasym = ~efficiencies[asym]*efficiencies2Lasym + efficiencies[sym]*~twoSingleInefficiencies + delta*~singleInefficiencies[0];
		}
		else
		{
			globalEfficiencies = ~efficiencies[sym]*globalEfficiencies + efficiencies[asym]*efficiencies2L2L - delta*efficiencies2Lasym;
			efficiencies2L2L = ~efficiencies[sym]*efficiencies2L2L + efficiencies[sym]*~twoSingleInefficiencies;
			efficiencies2Lasym = ~efficiencies[sym]*efficiencies2Lasym + efficiencies[asym]*~twoSingleInefficiencies - delta*~singleInefficiencies[1];
		}
		efficiencies2Lsym = ~efficiencies[sym]*efficiencies2Lsym + efficiencies[sym]*~singleInefficiencies[0];
		twoSingleInefficiencies *= ~efficiencies[loosestLeg];
		singleInefficiencies[0] *= ~efficiencies[sym];
		singleInefficiencies[1] *= ~efficiencies[asym];
	}
	return success;
}

template<typename Trig2L>
auto Calculator::globalEfficiency_Two2L(const LeptonList& leptons, unsigned runNumber, const Trig2L trig2L, const TrigEMU trigEMU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2L::is2L(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Two2L() at line " << __LINE__);
	Efficiencies efficiencies1L, efficiencies2L, efficiencies2Lor1L;
	bool success = globalEfficiency_One1L(leptons, runNumber, trigEMU.antiside<Trig2L>(), efficiencies1L);
	success = success && globalEfficiency_One2L(leptons, runNumber, trig2L, efficiencies2L);
	success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L, trigEMU.side<Trig2L>(), efficiencies2Lor1L);
	globalEfficiencies = efficiencies2L*~efficiencies1L + efficiencies1L*efficiencies2Lor1L;
	return success;
}

template<typename Trig2E, typename Trig2MU>
auto Calculator::globalEfficiency_Three2L(const LeptonList& leptons, unsigned runNumber, 
		const Trig2E trig2E, const Trig2MU trig2MU, const TrigEMU trigEMU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2E::is2L(xAOD::Type::Electron) && Trig2MU::is2L(xAOD::Type::Muon), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Three2L() at line " << __LINE__);
	Efficiencies efficiencies2L[2] = {{0.}, {0.}}, efficiencies2Lor1L[2] = {{0.}, {0.}};
	bool success = true;
	if(trig2E)
	{
		success = success && globalEfficiency_One2L(leptons, runNumber, trig2E, efficiencies2L[0]);
		if(trigEMU) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2E, trigEMU.side<Trig2E>(), efficiencies2Lor1L[0]);
		else efficiencies2Lor1L[0] = efficiencies2L[0];
	}
	else if(trigEMU) success = success && globalEfficiency_One1L(leptons, runNumber, trigEMU.side<Trig2E>(), efficiencies2Lor1L[0]);
	if(trig2MU)
	{
		success = success && globalEfficiency_One2L(leptons, runNumber, trig2MU, efficiencies2L[1]);
		if(trigEMU) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2MU, trigEMU.side<Trig2MU>(), efficiencies2Lor1L[1]);
		else efficiencies2Lor1L[1] = efficiencies2L[1];
	}
	else if(trigEMU) success = success && globalEfficiency_One1L(leptons, runNumber, trigEMU.side<Trig2MU>(), efficiencies2Lor1L[1]);
	globalEfficiencies = efficiencies2L[0]*~efficiencies2Lor1L[1] +  efficiencies2L[1]*~efficiencies2Lor1L[0] + efficiencies2Lor1L[0]*efficiencies2Lor1L[1];
	return success;
}

template<typename Trig2E, typename Trig2MU>
auto Calculator::globalEfficiency_Three2LSeveral1L(const LeptonList& leptons, unsigned runNumber, const Trig2E trig2E, const Trig2MU trig2MU, 
		const TrigEMU trigEMU, const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2E::is2L(xAOD::Type::Electron) && Trig2MU::is2L(xAOD::Type::Muon), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Three2LSeveral1L() at line " << __LINE__);
	Efficiencies efficiencies[4];
	bool success = true;
	if(trig2E) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2E, trigs1E, efficiencies[0]);
	else success = success && globalEfficiency_Several1L(leptons, runNumber, trigs1E, efficiencies[0]);
	if(trig2MU) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2MU, trigs1MU, efficiencies[1]);
	else success = success && globalEfficiency_Several1L(leptons, runNumber, trigs1MU, efficiencies[1]);
	if(trigEMU && !trigEMU.hiddenBy(trigs1E))
	{
		auto trigs1E_plusEMU = trigEMU.addTo(trigs1E);
		if(trig2E) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2E, trigs1E_plusEMU, efficiencies[2]);
		else success = success && globalEfficiency_Several1L(leptons, runNumber, trigs1E_plusEMU, efficiencies[2]);
	}
	else efficiencies[2] = efficiencies[0];
	if(trigEMU && !trigEMU.hiddenBy(trigs1MU))
	{
		auto trigs1MU_plusEMU = trigEMU.addTo(trigs1MU);
		if(trig2MU) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2MU, trigs1MU_plusEMU, efficiencies[3]);
		else success = success && globalEfficiency_Several1L(leptons, runNumber, trigs1MU_plusEMU, efficiencies[3]);
	}
	else efficiencies[3] = efficiencies[1];
	globalEfficiencies = Efficiencies(1.) - ~efficiencies[0]*~efficiencies[1] + (efficiencies[2]-efficiencies[0])*(efficiencies[3]-efficiencies[1]);
	return success;
}

template<typename Trig2L, typename Trig2Lsym, typename Trig1L>
auto Calculator::globalEfficiency_Six2LSeveral1L_singleObjectFactor(const LeptonList& leptons, unsigned runNumber, 
	const Trig2L trig2L, const Trig2Lsym trig2Lsym, const TrigEMU trigEMU1, const TrigEMU trigEMU2, const flat_set<Trig1L>& trigs1L, Efficiencies (&efficiencies)[4])
	-> typename std::enable_if<Trig2L::is2L() && Trig2Lsym::is2Lsym(Trig2L::object()) && Trig1L::is1L(Trig2L::object()), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Six2LSeveral1L_singleObjectFactor() at line " << __LINE__);
	auto eval_for = [=](const flat_set<Trig1L>& trigs1L_extended, Efficiencies& eff) -> bool
	{
		if(trig2L && trig2Lsym) return globalEfficiency_Two2LSeveral1L(leptons, runNumber, trig2L, trig2Lsym, trigs1L_extended, eff);
		else if(trig2L) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2L, trigs1L_extended, eff);
		else if(trig2Lsym) return globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2Lsym, trigs1L_extended, eff);
		else return globalEfficiency_Several1L(leptons, runNumber, trigs1L_extended, eff);
	};
	bool success = eval_for(trigs1L, efficiencies[0]);
	if(trigEMU1) success = success && eval_for(trigEMU1.addTo(trigs1L), efficiencies[1]);
	else efficiencies[1] = efficiencies[0];
	if(trigEMU2)
	{
		auto trigs1L_withEMU2 = trigEMU2.addTo(trigs1L);
		success = success && eval_for(trigs1L_withEMU2, efficiencies[2]);
		if(trigEMU1) success && eval_for(trigEMU1.addTo(trigs1L_withEMU2), efficiencies[3]);
		else efficiencies[3] = efficiencies[2];
	}
	else
	{
		efficiencies[2] = efficiencies[0];
		efficiencies[3] = efficiencies[1];
	}
	return success;
}

template<typename Trig2E, typename Trig2MU>
auto Calculator::globalEfficiency_Six2LSeveral1L(const LeptonList& leptons, unsigned runNumber, 
		const Trig2E trig2E, const Trig2Esym trig2Esym, const Trig2MU trig2MU, const Trig2MUsym trig2MUsym,
		const TrigEMU trigEMU1, const TrigEMU trigEMU2, const flat_set<Trig1E>& trigs1E, const flat_set<Trig1MU>& trigs1MU, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig2E::is2L(xAOD::Type::Electron) && Trig2MU::is2L(xAOD::Type::Muon), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Six2LSeveral1L() at line " << __LINE__);
	Efficiencies efficienciesE[4], efficienciesM[4];
	bool success = globalEfficiency_Six2LSeveral1L_singleObjectFactor(leptons, runNumber, trig2E, trig2Esym, trigEMU1, trigEMU2, trigs1E, efficienciesE)
		&& globalEfficiency_Six2LSeveral1L_singleObjectFactor(leptons, runNumber, trig2MU, trig2MUsym, trigEMU1, trigEMU2, trigs1MU, efficienciesM);
	globalEfficiencies = Efficiencies(1.) - ~efficienciesE[0]*~efficienciesM[0] + (efficienciesE[1]-efficienciesE[0])*(efficienciesM[1]-efficienciesM[0])
		+ (efficienciesE[2]-efficienciesE[0])*(efficienciesM[2]-efficienciesM[0]) 
		- (efficienciesE[0]-efficienciesE[1]-efficienciesE[2]+efficienciesE[3])*(efficienciesM[0]-efficienciesM[1]-efficienciesM[2]+efficienciesM[3]);
	return success;
}

template<typename Trig3Lmix>
auto Calculator::globalEfficiency_One3L(const LeptonList& leptons, unsigned runNumber, const Trig3Lmix trig, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig3Lmix::is3Lmix(), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_One3L() at line " << __LINE__);
	Efficiencies efficiencies[2] = {{0.}, {0.}};
	bool success = globalEfficiency_One2L(leptons, runNumber, trig.template side<Trig3Lmix::major_object()>(), efficiencies[0])
		&& globalEfficiency_One1L(leptons, runNumber, trig.template side<Trig3Lmix::minor_object()>(), efficiencies[1]);
	globalEfficiencies = efficiencies[0]*efficiencies[1];
	return success;
}

template<typename Trig3Lmix1, typename Trig3Lmix2>
auto Calculator::globalEfficiency_Two3L(const LeptonList& leptons, unsigned runNumber, const Trig3Lmix1 trig1, const Trig3Lmix2 trig2, Efficiencies& globalEfficiencies)
	-> typename std::enable_if<Trig3Lmix1::is3Lmix() && Trig3Lmix2::is3Lmix() && (Trig3Lmix1::major_object()!=Trig3Lmix2::major_object()), bool>::type
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Two3L() at line " << __LINE__);
	Efficiencies efficiencies[6] = {{0.}, {0.}, {0.}, {0.}, {0.}, {0.}};
	auto trig2La = trig1.template side<Trig3Lmix1::major_object()>();
	auto trig1La = trig2.template side<Trig3Lmix2::minor_object()>();
	bool success = globalEfficiency_One1L(leptons, runNumber, trig1La, efficiencies[0])
		&& globalEfficiency_One2L(leptons, runNumber, trig2La, efficiencies[1]);
	if(!trig2La.hiddenBy(trig1La)) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2La, trig1La, efficiencies[2]);
	else efficiencies[2] = efficiencies[0];
	auto trig2Lb = trig2.template side<Trig3Lmix2::major_object()>();
	auto trig1Lb = trig1.template side<Trig3Lmix1::minor_object()>();
	success = success && globalEfficiency_One1L(leptons, runNumber, trig1Lb, efficiencies[3])
		&& globalEfficiency_One2L(leptons, runNumber, trig2Lb, efficiencies[4]);
	if(!trig2Lb.hiddenBy(trig1Lb)) success = success && globalEfficiency_One2LSeveral1L(leptons, runNumber, trig2Lb, trig1Lb, efficiencies[5]);
	else efficiencies[5] = efficiencies[3];
	globalEfficiencies = efficiencies[0]*efficiencies[4] + efficiencies[3]*efficiencies[1] 
		+ (efficiencies[2]-efficiencies[0]-efficiencies[1])*(efficiencies[3]+efficiencies[4]-efficiencies[5]);
	return success;
}

bool Calculator::fillListOfLegsFor(const Lepton& lepton, const std::vector<TrigDef>& triggers, flat_set<std::size_t>& validLegs) const
{
	validLegs.clear();
	for(auto& trig : triggers)
	{
		bool success = true;
		if(lepton.type() == xAOD::Type::Electron)
		{
			switch(trig.type)
			{
			case TT_3E_ASYM:
				if(aboveThreshold(lepton, trig.leg[2])) validLegs.emplace(trig.leg[2]); // no break
			case TT_2E_ASYM: case TT_2E_MU_ASYM: case TT_3E_HALFSYM:
				if(aboveThreshold(lepton, trig.leg[1])) validLegs.emplace(trig.leg[1]); // no break
			case TT_SINGLE_E: case TT_2E_SYM: case TT_EMU: case TT_3E_SYM: 
			case TT_2E_MU_SYM: case TT_E_2MU_SYM: case TT_E_2MU_ASYM:
				if(aboveThreshold(lepton, trig.leg[0])) validLegs.emplace(trig.leg[0]);
				break;
			default:
				if(trig.type&TT_ELECTRON_FLAG) success = false;
			}
		}
		else if(lepton.type() == xAOD::Type::Muon)
		{
			switch(trig.type)
			{
			case TT_3MU_ASYM: 
				if(aboveThreshold(lepton, trig.leg[2])) validLegs.emplace(trig.leg[2]); // no break
			case TT_2MU_ASYM: case TT_3MU_HALFSYM:
				if(aboveThreshold(lepton, trig.leg[1])) validLegs.emplace(trig.leg[1]); // no break
			case TT_SINGLE_MU: case TT_2MU_SYM: case TT_3MU_SYM: 
				if(aboveThreshold(lepton, trig.leg[0])) validLegs.emplace(trig.leg[0]);
				break;
			case TT_E_2MU_ASYM:
				if(aboveThreshold(lepton, trig.leg[2])) validLegs.emplace(trig.leg[2]); // no break
			case TT_EMU: case TT_E_2MU_SYM: 
				if(aboveThreshold(lepton, trig.leg[1])) validLegs.emplace(trig.leg[1]);
				break;
			case TT_2E_MU_SYM: case TT_2E_MU_ASYM: 
				if(aboveThreshold(lepton, trig.leg[2])) validLegs.emplace(trig.leg[2]);
				break;
			default:
				if(trig.type&TT_MUON_FLAG) success = false;
			}
		}
		else if(lepton.type() == xAOD::Type::Photon)
		{
			switch(trig.type)
			{
			case TT_3PH_ASYM:
				if(aboveThreshold(lepton, trig.leg[2])) validLegs.emplace(trig.leg[2]); // no break
			case TT_2PH_ASYM: case TT_2E_MU_ASYM: case TT_3E_HALFSYM:
				if(aboveThreshold(lepton, trig.leg[1])) validLegs.emplace(trig.leg[1]); // no break
			case TT_SINGLE_PH: case TT_2PH_SYM: case TT_3PH_SYM:
				if(aboveThreshold(lepton, trig.leg[0])) validLegs.emplace(trig.leg[0]);
				break;
			default:
				if(trig.type&TT_PHOTON_FLAG) success = false;
			}
		}
		else
		{
			ATH_MSG_ERROR("Unknown lepton type");
			return false;
		}
		if(!success)
		{
			ATH_MSG_ERROR("Unrecognized trigger type " << trig.type);
			return false;
		}
	}
	return true;
}

bool Calculator::canTriggerBeFired(const TrigDef& trig, const std::vector<flat_set<std::size_t> >& firedLegs) const
{
	static_assert(std::tuple_size<decltype(trig.leg)>::value == 3, "extend Calculator::canTriggerBeFired() implementation to support triggers with >= 4 legs");
	int n0=0, n1=0, n0min = 1 + (trig.leg[1]!=0)*(trig.leg[0]!=trig.leg[1]?-5:1) + (trig.leg[2]!=0)*(trig.leg[0]!=trig.leg[2]?-9:1);
	if(n0min>0 || !trig.leg[2])
	{
		for(auto& legs : firedLegs)
		{
			bool fire0 = legs.count(trig.leg[0]);
			if(n0min <= 0) /// Asymmetric dilepton triggers
			{
				if(n1 && fire0) return true;
				if(legs.count(trig.leg[1]))
				{
					if(n0) return true;
					++n1;
				}
				if(fire0) ++n0;
			}
			else if(fire0 && ++n0>=n0min) return true; /// Single-lepton and symmetric di/trilepton triggers 
		}
	}
	else /// Trilepton triggers (except fully-symmetric ones that are addressed above)
	{
		auto end = firedLegs.end();
		for(auto legs0=firedLegs.begin();legs0!=end;++legs0)
		{
			for(int i=0;i<3;++i)
			{
				if(!legs0->count(trig.leg[i])) continue;
				for(auto legs1=legs0+1;legs1!=end;++legs1)
				{
					for(int j=1;j<3;++j)
					{
						if(!legs1->count(trig.leg[(i+j)%3])) continue;
						for(auto legs2=legs1+1;legs2!=end;++legs2)
						{
							if(legs2->count(trig.leg[(i+3-j)%3])) return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool Calculator::globalEfficiency_Toys(const LeptonList& leptons, unsigned runNumber, 
	const std::vector<TrigDef>& triggers, Efficiencies& globalEfficiencies)
{
	ATH_MSG_DEBUG("Entered Calculator::globalEfficiency_Toys() at line " << __LINE__);
	globalEfficiencies = {0.};
	if(m_parent->m_numberOfToys <= 0)
	{
		ATH_MSG_ERROR("The specified number of toys is <= 0");
		return false;
	}
	std::map<const Lepton*, std::vector<std::pair<std::size_t, Efficiencies> > > leptonEfficiencies;
	for(auto& lepton : leptons)
	{
		flat_set<std::size_t> validLegs;
		if(!fillListOfLegsFor(lepton, triggers, validLegs)) return false;
		auto& efficiencies = leptonEfficiencies[&lepton];
		const int nLegs = validLegs.size();
		if(nLegs)
		{
			bool success = true;
			for(std::size_t leg : m_parent->getSortedLegs(lepton, validLegs, success))
			{
				efficiencies.emplace_back(leg, getCachedTriggerLegEfficiencies(lepton, runNumber, leg, success));
			}
			if(!success) return false;
		}
	}
	unsigned long seed;
	if(!m_parent->m_useInternalSeed)
	{
		if(!m_parent->retrieveEventNumber(seed))
		{
			ATH_MSG_WARNING("Will use internal seed instead of event number");
			seed = m_parent->m_seed++;
		}
	}
	else seed = m_parent->m_seed++;
	std::mt19937_64 randomEngine(seed);
	std::uniform_real_distribution<double> uniformPdf(0., 1.);
	std::vector<flat_set<std::size_t> > firedLegs(leptonEfficiencies.size());
	unsigned long nPassed[2] = {0, 0};
	for(unsigned long toy=0;toy<m_parent->m_numberOfToys;++toy)
	{
		for(int step=0;step<2;++step) /// 0 = data, 1 = MC
		{
			auto legs = firedLegs.begin();
			for(auto& kv : leptonEfficiencies)
			{
				legs->clear();
				double x = uniformPdf(randomEngine);
				for(auto& p : kv.second)
				{
					if(x < (step? p.second.mc(): p.second.data())) legs->emplace(p.first);
				}
				++legs;
			}
			for(auto& trig : triggers)
			{
				if(!canTriggerBeFired(trig, firedLegs)) continue;
				++nPassed[step];
				break;
			}
		}
	}
	globalEfficiencies.data() = double(nPassed[0]) / double(m_parent->m_numberOfToys);
	globalEfficiencies.mc() = double(nPassed[1]) / double(m_parent->m_numberOfToys);
	return true;
}
