import os, re
import logging
msg = logging.getLogger('MetaReader')

def read_metadata(filenames, file_type=None, mode='lite', meta_key_filter= []):
	"""
	This tool is independent of Athena framework and returns the metadata from a given file.
	:param filenames: the input file from which metadata needs to be extracted.
	:param file_type: the type of file. POOL or BS (bytestream: RAW, DRAW) files.
	:param mode: if true, will return all metadata associated with the filename. By default, is false and this will
	return a "tiny" version which have only the following keys: 'file_guid', 'file_size', 'file_type', 'nentries'.
	:return: a dictionary of metadata for the given input file.
	"""

	# Check if the input is a file or a list of files.
	if isinstance(filenames, basestring):
		filenames = [filenames]

	# Check if file_type is an allowed value
	if file_type is not None:
		if file_type not in ('POOL', 'BS'):
			raise NameError('Allowed values for \'file_type\' parameter are: \'POOL\' or \'BS\': you provided "' + file_type + '"' )
		else:
			msg.info('Forced file_type: {0}'.format(file_type))

	# Check the value of mode parameter
	if mode not in ('tiny', 'lite', 'full'):
		raise NameError('Allowed values for \'mode\' parameter are: \'tiny\', \'lite\' or \'full\'')
	msg.info('Current mode used: {0}'.format(mode))

	if mode != 'full' and len(meta_key_filter) > 0:
		raise NameError('It is possible to use the meta_key_filter option only for full mode')
	if len(meta_key_filter) > 0:
		msg.info('Filter used: {0}'.format(meta_key_filter))

	# create the storage object for metadata.
	meta_dict = {}

	# ----- retrieve metadata from all filename or filenames --------------------------------------------------------#
	for filename in filenames:
		meta_dict[filename] = {}
		current_file_type = None
		# Determine the file_type of the input and store this information into meta_dict
		if not file_type:
			with open(filename, 'rb') as binary_file:
				magic_file = binary_file.read(4)

				if magic_file == 'root':
					current_file_type = 'POOL'
					meta_dict[filename]['file_type'] = 'POOL'

				else:
					current_file_type = 'BS'
					meta_dict[filename]['file_type'] = 'BS'

		else:
			current_file_type = file_type

		# add information about the file_size of the input filename
		meta_dict[filename]['file_size'] = os.path.getsize(filename)

		# ----- retrieves metadata from POOL files ------------------------------------------------------------------#
		if current_file_type == 'POOL':
			import ROOT
			# open the file using ROOT.TFile
			current_file = ROOT.TFile(filename)

			# open the tree 'POOLContainer' to read the number of entries
			meta_dict[filename]['nentries'] = current_file.Get('POOLContainer').GetEntriesFast()

			# read and add the 'GUID' value
			meta_dict[filename]['file_guid'] = _read_guid(filename)

			# ----- read extra metadata required for 'lite' and 'full' modes ----------------------------------------#
			if mode != 'tiny':
				# selecting from all tree the only one which contains metadata, respectively "MetaData"
				metadata_tree = current_file.Get('MetaData')
				# read all list of branches stored in "MetaData" tree
				metadata_branches = metadata_tree.GetListOfBranches()
				nr_of_branches = metadata_branches.GetEntriesFast()

				# object to store the names of metadata containers and their corresponding class name.
				meta_dict[filename]['metadata_items'] = {}

				# create a container for the list of filters used for the lite version
				meta_filter = []
				# set the filters for name
				if mode == 'lite':
					meta_filter = ['StreamAOD', 'EventStreamInfo_p3_StreamRDO', '_TagInfo']
				if mode == 'full' and len(meta_key_filter) > 0:
					meta_filter = [f.replace('/', '_') for f in meta_key_filter]
				# store all persistent classes for metadata container existing in a POOL/ROOT file.
				persistent_instances = {}

				for i in range(0, nr_of_branches):
					branch = metadata_branches.At(i)
					name = branch.GetName()

					class_name = branch.GetClassName()

					# fill the meta_dict with the name and the class_name of the object with metadata.
					# If the class name is IOVMetaDataContainer, replace the name of metadata container with
					# the name from 'folderName". All of this is done for consistency. ex. '_TagInfo' becomes '/TagInfo'
					meta_dict[filename]['metadata_items'][name.replace('_', '/')] = class_name

					if len(meta_filter) > 0:
						if name not in meta_filter:
							continue

					# assign the corresponding persistent class based of the name of the metadata container
					if class_name == 'EventStreamInfo_p3':
						persistent_instances[name] = ROOT.EventStreamInfo_p3()
					elif class_name == 'IOVMetaDataContainer_p1':
						persistent_instances[name] = ROOT.IOVMetaDataContainer_p1()
					elif class_name == 'xAOD::EventFormat_v1':
						persistent_instances[name] = ROOT.xAOD.EventFormat_v1()

					if name in persistent_instances:
						branch.SetAddress(ROOT.AddressOf(persistent_instances[name]))

				metadata_tree.GetEntry(0)

				# clean the meta-dict if the meta_key_filter flag is used, to return only the key of interest
				if len(meta_key_filter) > 0:
					meta_dict[filename] = {}

				# read the metadata
				for name, content in persistent_instances.items():
					key = name

					if hasattr(content, 'm_folderName'):
						key = getattr(content, 'm_folderName')

					meta_dict[filename][key] = _convert_value(content)


			# Filter the data and create a prettier output for the 'lite' mode
			if mode == 'lite':
				meta_dict = make_lite(meta_dict)

		# ----- retrieves metadata from bytestream (BS) files (RAW, DRAW) ------------------------------------------#
		elif current_file_type == 'BS':
			import eformat

			# store the number of entries
			bs = eformat.istream(filename)
			meta_dict[filename]['nentries'] = bs.total_events

			# store the 'guid' value
			data_reader = eformat.EventStorage.pickDataReader(filename)
			assert data_reader, 'problem picking a data reader for file [%s]' % filename

			if hasattr(data_reader, 'GUID'):
				meta_dict[filename]['file_guid'] = getattr(data_reader, 'GUID')()

			# if the flag full is set to true then grab all metadata
			# ------------------------------------------------------------------------------------------------------#
			if mode != "tiny":
				bs_metadata = {}

				for md in data_reader.freeMetaDataStrings():
					if md.startswith('Event type:'):
						k = 'evt_type'
						v = []
						if 'is sim' in md:
							v.append('IS_SIMULATION')
						else:
							v.append('IS_DATA')

						if 'is atlas' in md:
							v.append('IS_ATLAS')
						else:
							v.append('IS_TESTBEAM')

						if 'is physics' in md:
							v.append('IS_PHYSICS')
						else:
							v.append('IS_CALIBRATION')

						bs_metadata[k] = tuple(v)

					elif md.startswith('GeoAtlas:'):
						k = 'geometry'
						v = md.split('GeoAtlas:')[1].strip()
						bs_metadata[k] = v

					elif md.startswith('IOVDbGlobalTag:'):
						k = 'conditions_tag'
						v = md.split('IOVDbGlobalTag:')[1].strip()
						bs_metadata[k] = v

					elif '=' in md:
						k, v = md.split('=')
						bs_metadata[k] = v

				bs_metadata['runNumber'] = getattr(data_reader, 'runNumber')()
				bs_metadata['lumiblockNumber'] = getattr(data_reader, 'lumiblockNumber')()
				bs_metadata['projectTag'] = getattr(data_reader, 'projectTag')()
				bs_metadata['stream'] = getattr(data_reader, 'stream')()
				bs_metadata['beamType'] = getattr(data_reader, 'beamType')()
				bs_metadata['beamEnergy'] = getattr(data_reader, 'beamEnergy')()

				meta_dict[filename]['evt_type'] = bs_metadata.get('evt_type', [])
				meta_dict[filename]['geometry'] = bs_metadata.get('geometry', None)
				meta_dict[filename]['conditions_tag'] = bs_metadata.get('conditions_tag', None)

				# Promote up one level
				meta_dict[filename]['run_number'] = [bs_metadata.get('runNumber', None)]
				meta_dict[filename]['lumi_block'] = [bs_metadata.get('lumiblockNumber', None)]
				meta_dict[filename]['beam_type'] = [bs_metadata.get('beamType', None)]
				meta_dict[filename]['beam_energy'] = [bs_metadata.get('beamEnergy', None)]

				if not data_reader.good():
					# event-less file...
					meta_dict[filename]['run_number'].append(bs_metadata.get('run_number', 0))
					meta_dict[filename]['lumi_block'].append(bs_metadata.get('LumiBlock', 0))

				ievt = iter(bs)
				evt = ievt.next()
				evt.check()  # may raise a RuntimeError
				stream_tags = [dict(stream_type = tag.type, stream_name = tag.name, obeys_lbk = bool(tag.obeys_lumiblock)) for tag in evt.stream_tag()]
				meta_dict[filename]['stream_tags'] = stream_tags
				meta_dict[filename]['evt_number'] = [evt.global_id()]
				meta_dict[filename]['run_type'] = [eformat.helper.run_type2string(evt.run_type())]

				# fix for ATEAM-122
				if len(bs_metadata.get('evt_type', '')) == 0:  # see: ATMETADATA-6
					evt_type = ['IS_DATA', 'IS_ATLAS']
					if bs_metadata.get('stream', '').startswith('physics_'):
						evt_type.append('IS_PHYSICS')
					elif bs_metadata.get('stream', '').startswith('calibration_'):
						evt_type.append('IS_CALIBRATION')
					elif bs_metadata.get('projectTag', '').endswith('_calib'):
						evt_type.append('IS_CALIBRATION')
					else:
						evt_type.append('Unknown')

					meta_dict[filename]['evt_type'] = evt_type

				if mode == 'full':
					meta_dict[filename]['bs_metadata'] = bs_metadata

		# ------ Throw an error if the user provide other file types -------------------------------------------------#
		else:
			msg.error('Unknown filetype for {0} - there is no metadata interface for type {1}'.format(filename, current_file_type))
			return None

	return meta_dict


# Currently not used
# def _md5(fname, block_size=2**20, do_fast_md5=True):
#     import hashlib
#     # do_fast_md5 is for compatibility with AthFile.
#     # -- Sebastian Liem
#     hash_md5 = hashlib.md5()
#     with open(fname, 'rb') as f:
#         for chunk in iter(lambda: f.read(block_size), b''):
#             hash_md5.update(chunk)
#             if do_fast_md5:
#                 break
#     return hash_md5.hexdigest()


def _read_guid(filename):
	"""
	Extracts the "guid" (Globally Unique Identfier in POOL files and Grid catalogs) value from a POOL file.
	:param filename: the input file
	:return: the guid value
	"""
	import ROOT
	root_file = ROOT.TFile(filename)
	params = root_file.Get('##Params')

	regex = re.compile(r'^\[NAME\=([a-zA-Z0-9\_]+)\]\[VALUE\=(.*)\]')

	for i in range(params.GetEntries()):
		params.GetEntry(i)
		param = params.db_string

		result = regex.match(param)
		if result:
			name = result.group(1)
			value = result.group(2)

			if name == 'FID':
				return value

	return None


def _extract_fields(obj):
	result = {}

	for meth in dir(obj):
		if not meth.startswith('_'):
			if meth.startswith('m_'):

				field_name = str(meth)[2:]
				field_value = getattr(obj, meth)

				result[field_name] = _convert_value(field_value)

	return result


# compile the regex needed in _convert_value() outside it to optimize the code.
regex_cppname = re.compile(r'^([\w:]+)(<.*>)?$')
regex_persistent_class = re.compile(r'^([a-zA-Z]+_p\d+::)*[a-zA-Z]+_p\d+$')


def _convert_value(value):
	if hasattr(value, '__cppname__'):

		result = regex_cppname.match(value.__cppname__)

		if result:
			cpp_type = result.group(1)
			if cpp_type == 'vector':
				return [_convert_value(val) for val in value]

			elif cpp_type == 'pair':
				return _convert_value(value.first), _convert_value(value.second)

			# elif cpp_type == 'long':
			# 	return int(value)

			elif value.__cppname__ == "_Bit_reference":
				return bool(value)

			# special case which extracts data in a better format from IOVPayloadContainer_p1 class
			elif value.__cppname__ == 'IOVMetaDataContainer_p1':
				return _extract_fields_iovmdc(value)

			elif value.__cppname__ == 'IOVPayloadContainer_p1':
				return _extract_fields_iovpc(value)

			elif value.__cppname__ == 'xAOD::EventFormat_v1':
				return _extract_fields_ef(value)

			elif value.__cppname__ == 'EventType_p3':
				return _convert_event_type_bitmask( _extract_fields(value))

			elif regex_persistent_class.match(value.__cppname__):
				return _extract_fields(value)

	return value


def _extract_fields_iovmdc(value):
	return _convert_value(value.m_payload)


def _extract_fields_iovpc(value):
	result = {}

	for attr_idx in value.m_attrIndexes:
		name_idx = attr_idx.nameIndex()
		type_idx = attr_idx.typeIndex()
		obj_idx = attr_idx.objIndex()

		attr_name = value.m_attrName[name_idx]
		attr_value = None

		if type_idx == 0:
			attr_value = bool(value.m_bool[obj_idx])
		elif type_idx == 1:
			attr_value = int(value.m_char[obj_idx])
		elif type_idx == 2:
			attr_value = int(value.m_unsignedChar[obj_idx])
		elif type_idx == 3:
			attr_value = int(value.m_short[obj_idx])
		elif type_idx == 4:
			attr_value = int(value.m_unsignedShort[obj_idx])
		elif type_idx == 5:
			attr_value = int(value.m_int[obj_idx])
		elif type_idx == 6:
			attr_value = int(value.m_unsignedInt[obj_idx])
		elif type_idx == 7:
			attr_value = int(value.m_long[obj_idx])
		elif type_idx == 8:
			attr_value = int(value.m_unsignedLong[obj_idx])
		elif type_idx == 9:
			attr_value = long(value.m_longLong[obj_idx])
		elif type_idx == 10:
			attr_value = long(value.m_unsignedLongLong[obj_idx])
		elif type_idx == 11:
			attr_value = float(value.m_float[obj_idx])
		elif type_idx == 12:
			attr_value = float(value.m_double[obj_idx])
		elif type_idx == 13:
			# skipping this type because is file IOVPayloadContainer_p1.h (line 120) is commented and not considered
			pass
		elif type_idx == 14:
			attr_value = str(value.m_string[obj_idx])
		elif type_idx == 15:
			attr_value = long(value.m_date[obj_idx])
		elif type_idx == 16:
			attr_value = long(value.m_timeStamp[obj_idx])
		else:
			raise ValueError('Unknown type id {0} for attribute {1}'.format(type_idx, attr_name))

		if attr_name not in result:
			result[attr_name] = []
		result[attr_name].append(attr_value)

	max_element_count = 0
	for name, content in result.items():
		if len(content) > max_element_count:
			max_element_count = len(content)

	if max_element_count <= 1:
		for name, content in result.items():
			if len(content) > 0:
				result[name] = content[0]
			else:
				result[name] = None

	return result


def _extract_fields_ef(value):
	result = {}

	for ef_element in value:
		result[ef_element.first] = ef_element.second.className()

	return result


def _convert_event_type_bitmask(value):

	types = None
	for key in value:
		if key == 'bit_mask':
			val = value[key]

			bitmask_lenght = len(val)

			is_simulation = False
			is_testbeam = False
			is_calibration = False

			if bitmask_lenght > 0:  # ROOT.EventType.IS_SIMULATION
				is_simulation = val[0]

			if bitmask_lenght > 1:  # ROOT.EventType.IS_TESTBEAM
				is_testbeam = val[1]

			if bitmask_lenght > 2:  # ROOT.EventType.IS_CALIBRATION:
				is_calibration = val[2]

			types = [
				'IS_SIMULATION' if is_simulation else 'IS_DATA',
				'IS_TESTBEAM' if is_testbeam else 'IS_ATLAS',
				'IS_CALIBRATION' if is_calibration else 'IS_PHYSICS'
			]

	value['type'] = types

	return value


def make_lite(meta_dict):
	for filename, file_content in meta_dict.items():
		key_list = ['StreamAOD', 'EventStreamInfo_p3_StreamRDO']
		for key in key_list:
			if key in file_content and len(meta_dict[filename][key]) > 0:
				meta_dict[filename]['lumi_block'] = meta_dict[filename][key]['lumiBlockNumbers']
				meta_dict[filename]['run_number'] = meta_dict[filename][key]['runNumbers']
				meta_dict[filename]['mc_event_number'] = meta_dict[filename][key]['eventTypes'][0]['mc_event_number']
				meta_dict[filename]['mc_channel_number'] = meta_dict[filename][key]['eventTypes'][0]['mc_channel_number']
				meta_dict[filename]['event_types'] = meta_dict[filename][key]['eventTypes'][0]['type']
				meta_dict[filename].pop(key)

		if '/TagInfo' in file_content:
			meta_dict[filename]['beam_energy'] = float(meta_dict[filename]['/TagInfo']['beam_energy'])
			meta_dict[filename]['beam_type'] = meta_dict[filename]['/TagInfo']['beam_type']
			meta_dict[filename]['geometry'] = meta_dict[filename]['/TagInfo']['GeoAtlas']
			meta_dict[filename]['conditions_tag'] = meta_dict[filename]['/TagInfo']['IOVDbGlobalTag']
			meta_dict[filename].pop('/TagInfo')

	return meta_dict
