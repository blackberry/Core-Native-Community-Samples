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
 * libBBInvoke.h
 *
 *  Created on: 2014-05-09
 *      Author: eoros
 */

#ifndef LIBBBINVOKE_H_
#define LIBBBINVOKE_H_

#pragma GCC visibility push(default)

#ifdef __cplusplus
extern "C" {
#endif
void invoke(const char *action, const char *data,
		const char *file_transfer_mode, const char *id, const int list_id,
		const char *metadata, const char *source, const char *target,
		const int target_type_mask, const char *type, const char *uri);
#ifdef __cplusplus
}
#endif

#pragma GCC visibility pop

#endif /* LIBBBINVOKE_H_ */
