package jp.araobp.amg8833.analyzer

val MAX_VALUE = UByte.MAX_VALUE.toFloat()

fun minMaxNormalize(data: UByteArray): FloatArray {

    val floatArray = FloatArray(data.size)
    var min = UByte.MAX_VALUE.toFloat()
    var max = UByte.MIN_VALUE.toFloat()

    data.forEachIndexed { idx, byte ->
        val d = data[idx].toFloat()
        floatArray[idx] = d
        if (d > max) max = d
        if (d < min) min = d
    }

    val minmax = max - min

    floatArray.forEachIndexed { idx, float ->
        val f = floatArray[idx]
        floatArray[idx] = MAX_VALUE * (f - min) / minmax
    }

    return floatArray
}