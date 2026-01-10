#include "./transfer_auto_accept.hpp"

#include <solanaceae/object_store/meta_components_file.hpp>

#include <solanaceae/message3/components.hpp>
// for comp transfer tox filekind (TODO: generalize -> content system?)
#include <solanaceae/tox_messages/obj_components.hpp>

#include <solanaceae/util/config_model.hpp>

#include <filesystem>
#include <iostream>

TransferAutoAccept::TransferAutoAccept(ObjectStore2& os, RegistryMessageModelI& rmm, ConfigModelI& conf) : _os(os), _rmm(rmm), _conf(conf) {
	//_os.subscribe(this, ObjectStore_Event::object_update);

	_rmm.subscribe(this, RegistryMessageModel_Event::message_construct);
	_rmm.subscribe(this, RegistryMessageModel_Event::message_updated);

	if (!_conf.has_string("TransferAutoAccept", "save_path")) {
		_conf.set("TransferAutoAccept", "save_path", std::string_view{"tmp_save_dir"});
	}

	// dont load module instead?
	//if (!_conf.has_bool("TransferAutoAccept", "autoaccept")) {
		//_conf.set("TransferAutoAccept", "autoaccept", false); // safe default
	//}

	if (!_conf.has_int("TransferAutoAccept", "autoaccept_limit")) {
		_conf.set("TransferAutoAccept", "autoaccept_limit", int64_t(50l*1024l*1024l)); // sane default
	}
}

void TransferAutoAccept::iterate(void) {
	for (auto& it : _accept_queue) {
		if (it.all_of<ObjComp::Ephemeral::File::ActionTransferAccept>()) {
			continue; // already accepted
		}

		it.emplace<ObjComp::Ephemeral::File::ActionTransferAccept>(
			// TODO: contact to entry
			_conf.get_string("TransferAutoAccept", "save_path").value_or("tmp_save_dir"),
			false
		);
		std::cout << "TAA: auto accepted transfer\n";
		_os.throwEventUpdate(it);
		//_rmm.throwEventUpdate(it);
	}
	_accept_queue.clear();
}

void TransferAutoAccept::checkObj(ObjectHandle o) {
	if (!o) {
		return; // invalid object
	}

	if (o.all_of<ObjComp::Ephemeral::File::ActionTransferAccept>()) {
		return; // already accepted
	}

	if (o.all_of<ObjComp::F::TagLocalHaveAll>()) {
		return; // alreay have
	}

	//if (!h.all_of<Message::Components::Transfer::TagReceiving, Message::Components::Transfer::TagPaused, Message::Components::Transfer::FileInfo>()) {
	// TODO: tag receiving ??
	if (!o.all_of</*Message::Components::Transfer::TagReceiving, */ObjComp::Ephemeral::File::TagTransferPaused>()) {
		return;
	}

	if (!o.any_of<ObjComp::F::SingleInfo, ObjComp::F::CollectionInfo>()) {
		return; // dont know enough
	}

	// dont touch avatars for now
	// TODO: more generic file types??
	if (const auto* fk = o.try_get<ObjComp::Tox::FileKind>(); fk != nullptr && fk->kind != 0) {
		return;
	}

	const auto save_dir_path = _conf.get_string("TransferAutoAccept", "save_path").value_or("tmp_save_dir");
	uint64_t total_size {0u};
	if (const auto* si = o.try_get<ObjComp::F::SingleInfo>(); si != nullptr) {
		if (si->file_name.empty()) {
			return; // bad file
		}
		total_size = si->file_size;
		if (std::filesystem::exists(std::filesystem::path(save_dir_path) / si->file_name)) {
			return; // pre existing, dont auto
		}
	} else if (const auto* ci = o.try_get<ObjComp::F::CollectionInfo>(); ci != nullptr) {
		if (ci->file_list.empty() || ci->file_list.front().file_name.empty()) {
			return; // bad file
		}
		total_size = ci->total_size;

		return; // TODO: figure out a collection strategy
	}

	//const auto& file_info = h.get<Message::Components::Transfer::FileInfo>();
	// TODO: contact to entry
	if (total_size > uint64_t(_conf.get_int("TransferAutoAccept", "autoaccept_limit").value_or(1024*1024))) {
		return; // too large
	}

	_accept_queue.push_back(o);
}

void TransferAutoAccept::checkMsg(Message3Handle h) {
	if (!h.all_of<Message::Components::MessageFileObject>()) {
		return;
	}

	checkObj(h.get<Message::Components::MessageFileObject>().o);
}

bool TransferAutoAccept::onEvent(const Message::Events::MessageConstruct& e) {
	checkMsg(e.e);
	return false;
}

bool TransferAutoAccept::onEvent(const Message::Events::MessageUpdated& e) {
	checkMsg(e.e);
	return false;
}

bool TransferAutoAccept::onEvent(const ObjectStore::Events::ObjectUpdate&) {
	// too expensive rn
	//checkObj(e.e);
	return false;
}

