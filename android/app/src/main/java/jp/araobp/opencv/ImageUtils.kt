package jp.araobp.opencv

import org.opencv.core.CvType
import org.opencv.core.Mat

fun ByteArray.toUMat(rows: Int, cols: Int): Mat {
    val mat = Mat(rows, cols, CvType.CV_8U)
    mat.put(0, 0, this)
    return mat
}
