// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

package io.github.borked3ds.android.utils

class BiMap<K, V> {
    private val forward: MutableMap<K, V> = HashMap()
    private val backward: MutableMap<V, K> = HashMap()

    @Synchronized
    fun add(key: K, value: V) {
        forward[key] = value
        backward[value] = key
    }

    @Synchronized
    fun getForward(key: K): V? = forward[key]

    @Synchronized
    fun getBackward(key: V): K? = backward[key]
}
