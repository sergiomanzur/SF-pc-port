package com.madxbio97.syphonfilter

import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.madxbio97.syphonfilter.databinding.ActivityDossierBinding

class DossierActivity : AppCompatActivity() {

    private lateinit var binding: ActivityDossierBinding
    private var currentPage = 1
    private val totalPages = 4

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityDossierBinding.inflate(layoutInflater)
        setContentView(binding.root)

        showPage(currentPage)

        binding.btnPrev.setOnClickListener {
            if (currentPage > 1) {
                currentPage--
                showPage(currentPage)
            }
        }

        binding.btnNext.setOnClickListener {
            if (currentPage < totalPages) {
                currentPage++
                showPage(currentPage)
            }
        }
    }

    private fun showPage(page: Int) {
        val fileName = String.format("dossiers/screens/dossier_%02d.png", page)
        try {
            assets.open(fileName).use { stream ->
                val bitmap = BitmapFactory.decodeStream(stream)
                binding.ivDossier.setImageBitmap(bitmap)
            }
            binding.tvPage.text = " / "
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}
