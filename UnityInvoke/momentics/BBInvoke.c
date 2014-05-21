/**
 * Copyright 2014 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
/*
 * libBBInvoke.c
 *
 *  Created on: 2014-05-09
 *      Author: eoros
 */

#include <stdio.h>
#include <string.h>
#include <bps/navigator_invoke.h>

#include "public/BBInvoke.h"

void invoke(const char *action, const char *data,
		const char *file_transfer_mode, const char *id, const int list_id,
		const char *metadata, const char *source, const char *target,
		const int target_type_mask, const char *type, const char *uri) {

	navigator_invoke_invocation_t *iRequest = 0;
	navigator_invoke_invocation_create(&iRequest);

	if (action != 0) {
		navigator_invoke_invocation_set_action(iRequest, action);
	}

	if (data != 0) {
		navigator_invoke_invocation_set_data(iRequest, data, strlen(data));
	}

	if (id != 0) {
		navigator_invoke_invocation_set_id(iRequest, id);
	}

	if (list_id != 0) {
		navigator_invoke_invocation_set_list_id(iRequest, list_id);
	}

	if (metadata != 0) {
		navigator_invoke_invocation_set_metadata(iRequest, metadata);
	}

	if (source != 0) {
		navigator_invoke_invocation_set_source(iRequest, source);
	}

	if (target != 0) {
		navigator_invoke_invocation_set_target(iRequest, target);
	}

	if (target_type_mask != 0) {
		navigator_invoke_invocation_set_target_type_mask(iRequest,
				target_type_mask);
	}

	if (type != 0) {
		navigator_invoke_invocation_set_type(iRequest, type);
	}

	if (uri != 0) {
		navigator_invoke_invocation_set_uri(iRequest, uri);
	}

	/*
	 if (action != 0) { navigator_invoke_invocation_set_file_transfer_mode(iRequest, action); }
	 */

	navigator_invoke_invocation_send(iRequest);
	navigator_invoke_invocation_destroy(iRequest);
}
