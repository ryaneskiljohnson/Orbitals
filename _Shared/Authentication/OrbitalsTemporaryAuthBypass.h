/*
  ==============================================================================

    @fileoverview Single switch to bypass Cymasphere / NNAudio Access license checks in all Orbitals and Mandelbrot plugin editors.
    @module Orbitals/_Shared/Authentication

  ==============================================================================
*/

#pragma once

/**
 * @brief When 1 (default), `checkAuthorization()` in plugin editors returns true without reading license.dat.
 *
 * @note For production: set to 0 in this file, **or** define `ORBITALS_TEMPORARY_BYPASS_AUTHORIZATION=0`
 *       in Projucer preprocessor definitions for all configurations (overrides the default below).
 */
#ifndef ORBITALS_TEMPORARY_BYPASS_AUTHORIZATION
#define ORBITALS_TEMPORARY_BYPASS_AUTHORIZATION 1
#endif

namespace orbitals_auth_bypass_detail
{
/**
 * @brief @returns True if license verification should be skipped (development / staging).
 */
inline bool isBypassActive() noexcept
{
#if ORBITALS_TEMPORARY_BYPASS_AUTHORIZATION
    return true;
#else
    return false;
#endif
}
} // namespace orbitals_auth_bypass_detail
