// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#pragma once

/**
 * The result of a HTTP request precondition check.
 *
 * @see RFC 7232 section 3
 */
enum class PreconditionResult {
	/**
	 * The precondition header did not exist.
	 */
	NONE,

	/**
	 * Do the request (e.g. "If-Match" with a matching ETag or
	 * "If-None-Match" with a mismatching ETag).
	 */
	SUCCESS,

	/**
	 * Skip the request (e.g. "If-Match" with a mismatching ETag
	 * or "If-None-Match" with a matching ETag).
	 */
	FAILURE,
};
