# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
"""Instantiates TrigJetHypoToolConfig_flownetwork AlgTool 
from a hypo tree."""

from __future__ import print_function

from TrigHLTJetHypo.TrigHLTJetHypoConf import (
    TrigJetConditionConfig_abs_eta,
    TrigJetConditionConfig_signed_eta,
    TrigJetConditionConfig_et,
    TrigJetConditionConfig_dijet_mass,
    TrigJetConditionConfig_dijet_deta,
    TrigJetConditionConfig_dijet_dphi,
    TrigJetConditionConfig_moment,
    TrigJetConditionConfig_smc,
    TrigJetConditionConfig_compound,
    TrigJetHypoToolConfig_flownetwork,
    TrigJetHypoToolHelperMT,
    TrigJetHypoToolConfig_leaf,
    )

from TrigHLTJetHypoUnitTests.TrigHLTJetHypoUnitTestsConf import (
    AgreeHelperTool,
)

from TrigHLTJetHypo.node import Node

from collections import defaultdict

import copy


def is_leaf(node):
    return node.scenario in  ('simple', 'etaet', 'dijet')


def is_inner(node):
    return node.scenario in ('root', 'and', 'combgen', 'partgen')


class ConditionsToolSetterFlowNetwork(object):
    """Visitor to set instantiated AlgTools to a jet hypo tree"""
    
    def __init__(self, name):

        self.name = name
        # for simple, use TrigJetConditionConfig_etaet. Needs to be
        # completed because simple can conain any single jet condition
        self.tool_factories = {
            'eta': [TrigJetConditionConfig_abs_eta, 0], 
            'peta': [TrigJetConditionConfig_signed_eta, 0],
            'neta': [TrigJetConditionConfig_signed_eta, 0],
            'et': [TrigJetConditionConfig_et, 0],
            'djmass': [TrigJetConditionConfig_dijet_mass, 0],
            'djdphi': [TrigJetConditionConfig_dijet_dphi, 0],
            'djdeta': [TrigJetConditionConfig_dijet_deta, 0],
            'momwidth': [TrigJetConditionConfig_moment, 0],
            'smc': [TrigJetConditionConfig_smc, 0],
            'compound': [TrigJetConditionConfig_compound, 0],
            'flownetwork': [TrigJetHypoToolConfig_flownetwork, 0],
            'helper': [TrigJetHypoToolHelperMT, 0],
            }


        # map conaining parent child ids for the node
        self.treeMap = {0: 0}

        # map containing the a list of Condition factory AlgTools for scenario
        self.conditionMakers = defaultdict(list)

    
    def _set_conditions(self, node):
        """attach Conditions to leaf nodes"""

        self._mod_leaf(node)
        
        for cn in node.children:
            self._set_conditions(cn)


    def _remove_combgen(self, node):                       
        """Combination nodes represent parent children relationships.
        The child may be a subtree. For now, the parent will be in the 
        child list at position 0, and the child subtree in position 1."""

        parent_children = {}
        ipos  = 0

        # identify the combgen nodes, and rotate them
        for cn in node.children:
            if cn.scenario == 'combgen':
                assert (len(cn.children) == 2)
                parent_children[ipos] = cn.children
            ipos += 1

        # rotate the first combgen child (parent) into the position of the
        # combgen node, and set its child node.
        print ('parent_children ' + str(parent_children))
        print( node.children)
        for pos, p_c in parent_children.items():
            node.children[pos] = p_c[0]
            node.children[pos].children = [p_c[1]]

        for cn in node.children:
            self._remove_combgen(cn)

    def _remove_scenario(self, node, scenario):                       
        """Remove Partgen nodes by adding their children to their 
        parent's children."""

        def remove_scenario(node, scenario):
            for cn in node.children:
                if cn.scenario == scenario:
                    node.children.remove(cn)
                    node.children.extend(cn.children)
                    return True

            return False

        more = True
        while(more):
            more = remove_scenario(node, scenario)

        for cn in node.children:
            self._remove_scenario(cn, scenario)

  
    def _get_tool_instance(self, key, extra=''):
   
        klass = self.tool_factories[key][0]
        sn = self.tool_factories[key][1]
        
        name = '%s_%d' % (key, sn)
        if extra: name += '_' + extra
        tool = klass(name=name)            
        self.tool_factories[key][1] += 1
        return tool

    def _make_compound_condition_tools(self, node):

        #  compound_condition_tools:
        # elemental condition maker AlgToolshelper by the compound condition
        # AlgTool
        compound_condition_tools = []  
        for c in node.conf_attrs: # loop over conditions
            condition_tools = [] # elemental conditions for this compounnd ct.
            for k, v in c.items(): # loop over elemental conditions
                condition_tool = self._get_tool_instance(k)
                for lim, val in v.items():  # lim: min, max
                    setattr(condition_tool, lim, val)

                # SPECIAL CASE: Moment tool needs the name of the
                # moment as well as the min. max cuts:
                if (k.startswith ('mom')):
                    condition_tool.moment = k[len('mom'):]                    
                # END SPECIAL CASE

                condition_tools.append(condition_tool)

            # create compound condition from elemental condition
            compoundCondition_tool =self._get_tool_instance('compound')
            compoundCondition_tool.conditionMakers = condition_tools

            # add compound condition to list
            compound_condition_tools.append(compoundCondition_tool)

        return compound_condition_tools

    def _mod_leaf(self, node):
        """ Add Condition tools to For a leaf node."""

        if not is_leaf(node):
            return

        # parameters: (10et,0eta320)(20et)
        # conf_attrs: [2]: (is a list of length 2)
        # defaultdict(<type 'dict'>, {'et': {'max': 'inf', 'min': '10000.0'},
        #                             'eta': {'max': '3.2', 'min': '0.0'}})
        # defaultdict(<type 'dict'>, {'et': {'max': 'inf', 'min': '20000.0'}})


        # make a config tool and provide it with condition makers


        node.compound_condition_tools = self._make_compound_condition_tools(
            node)

    def _split_leaves(self, node):
        """Recursively replace leaf nodes with >1 Condition tools by nodes with
        one Condition tool."""

        def split_leaves(node):   
            for cn in node.children:
                if is_leaf(cn):
                    if len(cn.compound_condition_tools) > 1:
                        new_nodes =  []
                        for ct in cn.compound_condition_tools:
                            new_nodes.append(copy.deepcopy(cn))
                            new_nodes[-1].compound_condition_tools = [ct]
                            new_nodes[-1].conf_attrs = []
                        node.children.remove(cn)
                        node.children.extend(new_nodes)
                        return True # return after first modification

            return False


        more = True
        while(more):
            more = split_leaves(node)

        for cn in node.children:
            self._split_leaves(cn)


    def _find_shared(self, node, shared):

        if node.scenario == 'root':
            for cn in node.children:
                self._find_shared(cn, shared)

        elif node.scenario == 'and':
            for cn in node.children:
                shared.append([])
                self._find_shared(cn, shared)
                
        elif is_leaf(node):
            if len(node.children) == 0:
                if len(shared) == 0:
                    shared.append([node])
                else:
                    shared[-1].append(node)

            else:
                for cn in node.children:
                    self._find_shared(cn, shared)

        else:
            print ('illegal node: ', node.scenario)

        return shared


    def report(self):
        wid = max(len(k) for k in self.tool_factories.keys())
        rep = '\n%s: ' % self.__class__.__name__

        rep += '\n'.join(
            ['%s: %d' % (k.ljust(wid), v[1])
             for k, v in self.tool_factories.items()])

        return rep

    def _fill_tree_map(self, node, tmap):
        tmap[node.node_id] = node.parent_id

        for cn in node.children:
            self._fill_tree_map(cn, tmap)

            
    def _fill_conditions_map(self, node, cmap):
        if is_leaf(node):

            assert (len(node.compound_condition_tools) == 1)
            cmap[node.node_id] = node.compound_condition_tools[0]

        else:
            cmap[node.node_id] = None
        
        for cn in node.children:
            self._fill_conditions_map(cn, cmap)


    def _map_2_vec(self, amap):

        vec = [0 for i in range(len(amap))]
        for nid, value in amap.items():
            vec[nid] = value
        return vec

    def _check_scenarios(self, node):
        if not(is_inner(node) or is_leaf(node)):
            raise RuntimeError(
                'ConditionsToolSetter: illegal scenario: %s' % node.scenario)

        for cn in node.children:
            self._check_scenarios(cn)

    def mod(self, node):
        """Entry point for this module. 
        Modifies a  (usually compound) hypo tree node to 
        reduce it to form from whuch the treevector, conditionsVector and
        sharedNodes list can be extracted. These will be used to initialise
        FlowNetworkBuilder.
        """

        # navigate the tree filling in node-parent and node- Condtion factory
        # relations

        print ('%s: starts' % self.__class__.__name__)


        print ('%s: start step 1' % self.__class__.__name__)

        # Alg step 1: add root node
        root = Node(scenario='root');
        root.children = [node]

        print ('%s: checking scenarios' % self.__class__.__name__)

        self._check_scenarios(root)
        
        print ('%s: setting conditions' % self.__class__.__name__)
        # add Condition builders to leaf nodes.
        self._set_conditions(root)
        
        print ('%s: removing combgen' % self.__class__.__name__)
        # Alg step 2: remove combgen nodes
        self._remove_combgen(root)

        print ('%s: split leaves' % self.__class__.__name__)
        # Alg step 3: split leaf nodes with multiple Conditions with a
        # single Condition
        self._split_leaves(root)
        
        print ('%s: remove partgen' % self.__class__.__name__)
        # Alg step 4: remove partgen nodes
        # single Condition
        self._remove_scenario(root, 'partgen')

        # Alg step 5: identify the leaf nodes that are to shared
        # ie that see the input jet collection. Then remove And nodes
        shared = []
        self._find_shared(root, shared)
        self._remove_scenario(root, 'and')

        root.set_ids(node_id=0, parent_id = 0)
        
        self.shared = []
        for slist in shared:
            self.shared.append([n.node_id for n in slist])
        tree_map = {}
        self._fill_tree_map(root, tree_map)
        self.treeVec = self._map_2_vec(tree_map)

        conditionsMap = {}
        self._fill_conditions_map(root, conditionsMap)
        self.conditionsVec = self._map_2_vec(conditionsMap)
               
