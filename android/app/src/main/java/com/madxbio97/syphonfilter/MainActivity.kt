package com.madxbio97.syphonfilter

import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import org.libsdl.app.SDLActivity
import java.io.File
import java.io.FileOutputStream

class MainActivity : SDLActivity() {

    companion object {
        private const val REQUEST_CODE_CUE = 1001
    }

    private var selectedCuePath: String? = null

    override fun getMainSharedObject(): String {
        return "libsyphon_filter.so"
    }

    override fun getLibraries(): Array<String> {
        return arrayOf("syphon_filter")
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        extractAssetsIfNeeded()

        val savedImage = loadSavedGameImagePath()
        if (savedImage != null && File(savedImage).exists()) {
            selectedCuePath = savedImage
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == REQUEST_CODE_CUE && resultCode == RESULT_OK) {
            val uri = data?.data
            if (uri != null) {
                val path = copyUriToInternalStorage(uri)
                selectedCuePath = path
                saveGameImagePath(path)
                showLaunchDialog()
            }
        }
    }

    private fun pickCueFile() {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).apply {
            addCategory(Intent.CATEGORY_OPENABLE)
            type = "*/*"
        }
        startActivityForResult(intent, REQUEST_CODE_CUE)
    }

    override fun getArguments(): Array<String> {
        val args = mutableListOf<String>()
        var cue = selectedCuePath ?: loadSavedGameImagePath()
        if (cue == null) {
            val candidates = listOf(
                "/sdcard/Download/game.cue",
                "/sdcard/Download/Syphon Filter (v1.1).cue",
                File(filesDir, "game.cue").absolutePath
            )
            for (candidate in candidates) {
                if (File(candidate).exists()) {
                    cue = candidate
                    break
                }
            }
        }
        if (cue != null) {
            args.add(cue)
        }
        return args.toTypedArray()
    }

    private fun showLaunchDialog() {
        val options = arrayOf(
            getString(R.string.launch_game),
            getString(R.string.select_disc),
            getString(R.string.graphics_settings),
            getString(R.string.dossiers)
        )

        AlertDialog.Builder(this)
            .setTitle(R.string.app_name)
            .setItems(options) { _, which ->
                when (which) {
                    0 -> {
                        if (selectedCuePath == null) {
                            Toast.makeText(this, R.string.select_disc, Toast.LENGTH_LONG).show()
                            pickCueFile()
                        }
                    }
                    1 -> pickCueFile()
                    2 -> startActivity(Intent(this, SettingsActivity::class.java))
                    3 -> startActivity(Intent(this, DossierActivity::class.java))
                }
            }
            .setCancelable(false)
            .show()
    }

    private fun extractAssetsIfNeeded() {
        val localesDir = File(filesDir, "locales/ru-vit")
        val skyboxDir = File(filesDir, "assets/skyboxes")
        if (!localesDir.exists()) localesDir.mkdirs()
        if (!skyboxDir.exists()) skyboxDir.mkdirs()

        copyAssetFolder("locales", File(filesDir, "locales"))
        copyAssetFolder("skyboxes", File(filesDir, "assets/skyboxes"))
    }

    private fun copyAssetFolder(assetPath: String, targetDir: File) {
        try {
            val list = assets.list(assetPath) ?: return
            if (list.isEmpty()) {
                val targetFile = File(targetDir, File(assetPath).name)
                if (!targetFile.exists()) {
                    assets.open(assetPath).use { input ->
                        FileOutputStream(targetFile).use { output ->
                            input.copyTo(output)
                        }
                    }
                }
            } else {
                targetDir.mkdirs()
                for (file in list) {
                    val subPath = if (assetPath.isEmpty()) file else "$assetPath/$file"
                    val subList = assets.list(subPath)
                    if (subList.isNullOrEmpty()) {
                        val targetFile = File(targetDir, file)
                        if (!targetFile.exists()) {
                            assets.open(subPath).use { input ->
                                FileOutputStream(targetFile).use { output ->
                                    input.copyTo(output)
                                }
                            }
                        }
                    } else {
                        copyAssetFolder(subPath, File(targetDir, file))
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun copyUriToInternalStorage(uri: Uri): String {
        val fileName = "game.cue"
        val file = File(filesDir, fileName)
        contentResolver.openInputStream(uri)?.use { input ->
            FileOutputStream(file).use { output ->
                input.copyTo(output)
            }
        }
        return file.absolutePath
    }

    private fun saveGameImagePath(path: String) {
        val dir = File(filesDir, "SyphonFilterPC")
        if (!dir.exists()) dir.mkdirs()
        val ini = File(dir, "launcher.ini")
        val text = if (ini.exists()) ini.readText() else ""
        ini.writeText(text + "\n[Game]\nImage=\n")
    }

    private fun loadSavedGameImagePath(): String? {
        val ini = File(File(filesDir, "SyphonFilterPC"), "launcher.ini")
        if (!ini.exists()) return null
        for (line in ini.readLines()) {
            if (line.startsWith("Image=")) {
                return line.substring("Image=".length).trim()
            }
        }
        return null
    }
}
