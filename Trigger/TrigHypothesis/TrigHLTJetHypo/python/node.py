# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from constants import logicals
    
class Node(object):
    
    def __init__(self, scenario):

        self.node_id = None
        self.parent_id = None
        
        self.scenario = scenario
        self.parameters = ''
        self.children = []
        self.conf_attrs = {}
        self.tool = None

        # self.tree_top kludge carensure top level tools get chain name
        # as Tool name
        self.tree_top = False

    def set_ids(self, node_id, parent_id):
        "Set ids of nodes in a tree"

        self.node_id = node_id
        self.parent_id = parent_id

        c_node_id = self.node_id + 1
        for c in self.children:
            c.set_ids(c_node_id, self.node_id)
            c_node_id += 1

    def add_child(self, c):
        self.children.append(c)
        
    def accept(self, modifier):
        "call self before children"

        for c in self.children:
            c.accept(modifier)
        modifier.mod(self)

    def accept_cf(self, modifier):
        "call children before self"
        
        for c in self.children:
            c.accept_cf(modifier)
        modifier.mod(self)


    def buildTree(self, treeVisitor):
        if self.children:
            if self.scenario in logicals:
                treeVisitor.add(self.scenario + '(')
            else: 
                treeVisitor.add(self.tool.name() + '(')
                
            for c in self.children:
                c.buildTree(treeVisitor)
            treeVisitor.add(') ')

        else:
            treeVisitor.add(self.tool.name() + ' ')
        
    def dump(self, n_in=0):
        indent = ' '*n_in
        s = '\n'
        s +=  indent + 'Node. scenario: %s \n' % self.scenario
        s +=  indent + 'node id: %s \n' % self.node_id
        s +=  indent + 'parent node id: %s \n' % self.parent_id
        s +=  indent + 'is tree top? %s \n' % self.tree_top
        s += indent + 'parameters: %s\n' % str(self.parameters)
        s += indent + 'conf_attrs: %s\n' % str(self.conf_attrs)
        s += indent + 'AlgTool: %s\n' % str(self.tool)
        s += indent + 'No of children: %d\n\n' % len(self.children)
        for c in self.children:
            s += c.dump(n_in+5)

        return s
        
