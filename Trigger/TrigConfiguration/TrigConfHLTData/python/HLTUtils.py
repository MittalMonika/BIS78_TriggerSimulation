#
#  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
#

def string2hash(string):
  """Hashes a string using the HLT's HLTUtils::string2hash hashing function. Python implimentation. """
  hash = 0xd2d84a61L
  mask = 0xffffffffL
  for character in reversed(string):
    hash ^= (( (((hash >> 5) & mask) + ord(character)) & mask) + ((hash << 7) & mask)) & mask
  for character in string:
    hash ^= (( (((hash >> 5) & mask) + ord(character)) & mask) + ((hash << 7) & mask)) & mask
  return hash
